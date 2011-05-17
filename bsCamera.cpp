#include "bsCamera.h"

#include <memory>

#include "bsMath.h"
#include "bsSceneGraph.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsNodeCollectorPhantom.h"
#include "bsDx11Renderer.h"
#include "bsConstantBuffers.h"


bsCamera::bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph,
	bsHavokManager* havokManager)
	: mProjectionNeedsUpdate(false)
	, mViewNeedsUpdate(true)
	, mViewProjectionNeedsUpdate(true)
	
	, mProjectionInfo(projectionInfo)

	, mSceneGraph(sceneGraph)
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
	hkpRigidBodyCinfo cInfo;
	cInfo.m_shape = shape;
	cInfo.m_enableDeactivation = false;
	cInfo.m_motionType = hkpMotion::MOTION_KEYFRAMED;
	mRigidBody = new hkpRigidBody(cInfo);
	havokManager->getGraphicsWorld()->addEntity(mRigidBody);


	setPosition(hkVector4(0.0f, 0.0f, -125.0f));
}

bsCamera::~bsCamera()
{
	mViewProjectionBuffer->Release();
	mPhantom->removeReference();
	mRigidBody->removeReference();
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
	translation.sub3clobberW(mTransform.getTranslation());

	translate(translation);
}
void bsCamera::translate(const hkVector4& translation)
{
	//Generate the new position
	hkVector4 position(mTransform.getTranslation());
	position.sub3clobberW(translation);

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
	rotatedTransform.getTranslation().mul4(-1.0f);

	mPhantom->setTransform(rotatedTransform);
	mRigidBody->setTransform(rotatedTransform);
}
