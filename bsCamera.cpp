#include "StdAfx.h"

#include "bsCamera.h"

#include <memory>

#include <Physics/Internal/BroadPhase/TreeBroadPhase/hkpTreeBroadPhase.h>

#include "bsMath.h"
#include "bsSceneGraph.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsNodeCollectorPhantom.h"
#include "bsDx11Renderer.h"
#include "bsConstantBuffers.h"


// Compute a plane equation from triangle vertices, store offset to the origin in W.
hkVector4 computePlaneFromTriangle(const hkVector4& vertex1, const hkVector4& vertex2,
	const hkVector4& vertex3)
{
	hkVector4 v12;
	v12.setSub(vertex2, vertex1);
	hkVector4 v13;
	v13.setSub(vertex3, vertex1);

	hkVector4 plane;
	plane.setCross(v12, v13);
	plane.normalize<3>();
	plane.setW(-plane.dot<3>(vertex1));

	return plane;
}


bsCamera::bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph,
	bsHavokManager* havokManager)
	: mProjectionNeedsUpdate(false)
	, mViewNeedsUpdate(true)
	, mViewProjectionNeedsUpdate(true)
	
	, mProjectionInfo(projectionInfo)

	, mSceneGraph(sceneGraph)
	, mHybridBroadphase(static_cast<const hkpTreeBroadPhase*>(havokManager->getGraphicsWorld()->getBroadPhase()))
	, mDeviceContext(mSceneGraph->getRenderer()->getDeviceContext())

	, mTransform(hkTransform::getIdentity())
	, mRotationX(0.0f)
	, mRotationY(0.0f)
{
	BS_ASSERT(sceneGraph);
	BS_ASSERT(havokManager);
	BS_ASSERT(havokManager->getGraphicsWorld() != nullptr);

	XMStoreFloat4x4(&mProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&mViewProjection, XMMatrixIdentity());

	ID3D11Device* device = mSceneGraph->getRenderer()->getDevice();

	//Create view projection buffer
	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBCamera);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;


	if (FAILED(device->CreateBuffer(&bufferDescription, nullptr, &mViewProjectionBuffer)))
	{
		BS_ASSERT(!"Failed to create view projection buffer");
	}
	mDeviceContext->VSSetConstantBuffers(0, 1, &mViewProjectionBuffer);

	bsMath::XMFloat4x4PerspectiveFovLH(mProjectionInfo.mFieldOfView,
		mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip,
		mProjection);


	constructFrustum();

	hkVector4 vertices[8];
	for (unsigned int i = 0; i < 8; ++i)
	{
		bool negX = i > 1;//First two
		bool negY = i & 1;//Odd/even
		bool zeroZ = i < 4;//First 4
		if (zeroZ)
		{
			vertices[i].set(negX ? -2.0f : 2.0f, negY ? -2.0f : 2.0f, mProjectionInfo.mNearClip);
		}
		else
		{
			bool even = (i & 1) == 0;
			bool last2 = i > 5;

			float viewLength = mProjectionInfo.mFarClip - mProjectionInfo.mNearClip;
			float farHeight = 2 * tanf(XMConvertToRadians(mProjectionInfo.mFieldOfView) * 0.5f) * viewLength;
			float farWidth = farHeight * mProjectionInfo.mAspectRatio;
			
			vertices[i].set(even ? farWidth : -farWidth, last2 ? farHeight : -farHeight,
				mProjectionInfo.mFarClip);
		}
	}


	hkTransform transform;
	transform.setIdentity();

	hkStridedVertices stridedVertices(vertices, 8);
	hkpConvexVerticesShape* shape = new hkpConvexVerticesShape(stridedVertices);
	
	mPhantom = new bsNodeCollectorPhantom(shape, transform);
	havokManager->getGraphicsWorld()->addPhantom(mPhantom);
	
	//TODO: Remove at some point. The RB's only purpose is to make sure stuff looks right
	//in the VDB.
	/*
	hkpRigidBodyCinfo cInfo;
	cInfo.m_shape = shape;
	cInfo.m_enableDeactivation = false;
	cInfo.m_motionType = hkpMotion::MOTION_KEYFRAMED;
	mRigidBody = new hkpRigidBody(cInfo);
	havokManager->getGraphicsWorld()->addEntity(mRigidBody);
	*/
}

bsCamera::~bsCamera()
{
	mViewProjectionBuffer->Release();
	mPhantom->removeReference();
	//mRigidBody->removeReference();
}

void bsCamera::update()
{
	if (mViewNeedsUpdate)
	{
		updateView();
	}
	if (mProjectionNeedsUpdate)
	{
		updateProjection();
	}
	if (mViewProjectionNeedsUpdate)
	{
		updateViewProjection();
	}
}

void bsCamera::updateView()
{
	mViewNeedsUpdate = false;
	//View projection matrix needs to be rebuilt.

	//const hkVector4 currentTranslation = mTransform.getTranslation();
	hkRotation rotationX;
	rotationX.setAxisAngle(hkVector4(1.0f, 0.0f, 0.0f), mRotationY);
	hkRotation rotationY;
	rotationY.setAxisAngle(hkVector4(0.0f, 1.0f, 0.0f), mRotationX);

	rotationX.mul(rotationY);
	mTransform.setRotation(rotationX);

	updatePhantomTransform();

	mViewProjectionNeedsUpdate = true;
}

void bsCamera::updateProjection()
{
	bsMath::XMFloat4x4PerspectiveFovLH(mProjectionInfo.mFieldOfView, mProjectionInfo.mAspectRatio,
		mProjectionInfo.mNearClip, mProjectionInfo.mFarClip, mProjection);

	mProjectionNeedsUpdate = false;
	//View projection matrix needs to be rebuilt.
	mViewProjectionNeedsUpdate = true;
}

void bsCamera::updateViewProjection()
{
	hkTransform transform(mTransform);
	hkVector4& translation = transform.getTranslation();
	translation.setRotatedDir(transform.getRotation(), translation);
	
	bsMath::XMFloat4x4Multiply(bsMath::toXM(transform), mProjection, mViewProjection);
	bsMath::XMFloat4x4Transpose(mViewProjection);

	CBCamera cbCamera;
	cbCamera.view = bsMath::toXM(transform);
	cbCamera.projection = mProjection;
	cbCamera.viewProjection = mViewProjection;
	bsMath::XMFloat4x4Transpose(cbCamera.view);
	bsMath::XMFloat4x4Transpose(cbCamera.projection);
	cbCamera.cameraPosition = bsMath::toXM4(mTransform.getTranslation());

	XMVECTOR determinant;
	XMStoreFloat4x4(&cbCamera.inverseViewProjection, XMMatrixInverse(&determinant, XMLoadFloat4x4(&mViewProjection)));

	mDeviceContext->UpdateSubresource(mViewProjectionBuffer, 0, nullptr, &cbCamera, 0, 0);
	
	mViewProjectionNeedsUpdate = false;
}

void bsCamera::rotateAboutAxis(const hkVector4& axis, float degrees)
{
	float radians = XMConvertToRadians(degrees);

	hkRotation rot;
	rot.setAxisAngle(axis, radians);
	mTransform.getRotation().mul(rot);

	updatePhantomTransform();

	mViewNeedsUpdate = true;
}

void bsCamera::lookAt(const hkVector4& targetPosition)
{
	BS_ASSERT(!"bsCamera::lookAt not implemented");

	mViewNeedsUpdate = true;
}

void bsCamera::setPosition(const hkVector4& position)
{
	//Get the translation based on current position and new position
	hkVector4 translation(position);
	translation.sub(mTransform.getTranslation());

	translate(translation);
}
void bsCamera::translate(const hkVector4& translation)
{
	//Generate the new position
	hkVector4 position(mTransform.getTranslation());
	position.sub(translation);

	mTransform.setTranslation(position);

	updatePhantomTransform();
	
	mViewNeedsUpdate = true;
}

void bsCamera::rotateX(float angleRadians)
{
	mRotationX += angleRadians;

	mViewNeedsUpdate = true;
}

void bsCamera::rotateY(float angleRadians)
{
	mRotationY += angleRadians;

	mViewNeedsUpdate = true;
}

void bsCamera::updatePhantomTransform()
{
	hkTransform rotatedTransform(mTransform);
	rotatedTransform.getRotation().invert(0.00001f);
	rotatedTransform.getTranslation().mul(hkSimdReal::convert(-1.0f));

	mPhantom->setTransform(rotatedTransform);
	//mRigidBody->setTransform(rotatedTransform);
}

std::vector<bsSceneNode*> bsCamera::getVisibleSceneNodes() const
{
	//const_cast<bsCamera*>(this)->constructFrustum();





	return mPhantom->getOverlappingSceneNodes();
}
#include "bsBroadphaseHandle.h"
void bsCamera::constructFrustum()
{
	//Construct the corners of the near clip plane
	hkVector4 nearPlane[4];
	nearPlane[0].set( 2.0f,  2.0f, mProjectionInfo.mNearClip);
	nearPlane[1].set( 2.0f, -2.0f, mProjectionInfo.mNearClip);
	nearPlane[2].set(-2.0f,  2.0f, mProjectionInfo.mNearClip);
	nearPlane[3].set(-2.0f, -2.0f, mProjectionInfo.mNearClip);

	//View distance
	const float viewLength = mProjectionInfo.mFarClip - mProjectionInfo.mNearClip;
	//Height of the far clip plane
	const float farHeight = 2 * tanf(XMConvertToRadians(mProjectionInfo.mFieldOfView) * 0.5f) * viewLength;
	//Width of the far clip plane
	const float farWidth = farHeight * mProjectionInfo.mAspectRatio;

	//Corners of far clip plane
	hkVector4 farPlane[4];
	farPlane[0].set( farWidth, -farHeight, mProjectionInfo.mFarClip);
	farPlane[1].set(-farWidth, -farHeight, mProjectionInfo.mFarClip);
	farPlane[2].set( farWidth,  farHeight, mProjectionInfo.mFarClip);
	farPlane[3].set(-farWidth,  farHeight, mProjectionInfo.mFarClip);


	//Transform the vertices
	const hkRotation& currentRotation(mTransform.getRotation());
	hkVector4 direction(0.0f, 0.0f, 1.0f, 0.0f);
	direction.setRotatedDir(currentRotation, direction);

	for (unsigned int i = 0; i < 4; ++i)
	{
		nearPlane[i].setRotatedDir(currentRotation, nearPlane[i]);
		farPlane[i].setRotatedDir(currentRotation, farPlane[i]);
	}


	hkVector4 planes[6];
	//Side planes
	planes[0] = computePlaneFromTriangle(nearPlane[0], nearPlane[1], farPlane[0]);
	planes[1] = computePlaneFromTriangle(nearPlane[1], nearPlane[2], farPlane[1]);
	planes[2] = computePlaneFromTriangle(nearPlane[2], nearPlane[3], farPlane[2]);
	planes[3] = computePlaneFromTriangle(nearPlane[3], nearPlane[0], farPlane[3]);

	//Near plane
	planes[4] = computePlaneFromTriangle(nearPlane[0], nearPlane[2], farPlane[1]);

	//Far plane
	planes[5] = computePlaneFromTriangle(farPlane[0], farPlane[1], farPlane[2]);


	//Do the culling
	hkArray<const hkpBroadPhaseHandle*> handles;
	handles.reserve(4096 / sizeof(const hkpBroadPhaseHandle*));
	mHybridBroadphase->queryConvex(planes, 6, handles, static_cast<hkUint32>(hkpTreeBroadPhase::ALL_GROUPS));

	std::vector<const hkpBroadPhaseHandle*> hkHandles(handles.begin(), handles.end());

	std::vector<const bsBroadphaseHandle*> bsHandles(handles.getSize());
	for (int i = 0; i < handles.getSize(); ++i)
	{
		bsHandles[i] = static_cast<const bsBroadphaseHandle*>(handles[i]);
	}

	for (int i = 0; i < handles.getSize(); ++i)
	{
		auto handle = mHybridBroadphase->getHandleFromObject(handles[i]);
		i = i;
	}
	
	//mHybridBroadphase->getHandleFromObject(hkHandles[0]).
}
