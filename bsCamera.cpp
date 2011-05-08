#include "bsCamera.h"

#include <cassert>
#include <memory>

#include "bsSceneGraph.h"
#include "bsLog.h"
#include "bsHavokManager.h"
#include "bsNodeCollectorPhantom.h"
#include "bsDx11Renderer.h"
#include "bsConstantBuffers.h"


bsCamera::bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph,
	bsHavokManager* havokManager)
	: mProjectionNeedsUpdate(true)
	, mViewNeedsUpdate(true)
	, mViewProjectionNeedsUpdate(true)
	
	, mProjectionInfo(projectionInfo)

	, mSceneGraph(sceneGraph)
	, mDeviceContext(mSceneGraph->getRenderer()->getDeviceContext())
	, mTransform(hkTransform::getIdentity())
	, mRotationX(0.0f)
	, mRotationY(0.0f)
{
	assert(sceneGraph);
	assert(havokManager);
	assert(havokManager->getGraphicsWorld() != nullptr);

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
		bsLog::logMessage("bsCamera::bsCamera: failed to create view projection buffer",
			pantheios::SEV_CRITICAL);
		assert(!"bsCamera::bsCamera: failed to create view projection buffer");
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
	//havokManager->getGraphicsWorld()->addEntity(mRigidBody);


	translate(0.0f, 0.0f, -125.0f);
}

bsCamera::~bsCamera()
{
	mViewProjectionBuffer->Release();
	mPhantom->removeReference();
	mRigidBody->removeReference();
}

inline void bsCamera::lookAt(const XMFLOAT3& position)
{
	assert(!"bsCamera::lookAt not implemented");

	mViewNeedsUpdate = true;
}

inline void bsCamera::setPosition(float x, float y, float z)
{
	assert(!"bsCamera::setPosition not implemented");

	mViewNeedsUpdate = true;
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


	hkTransform rotatedTransform(mTransform);
	rotatedTransform.getRotation().invert(0.00001f);
	rotatedTransform.getTranslation().mul4(-1.0f);

	mPhantom->setTransform(rotatedTransform);
	mRigidBody->setTransform(rotatedTransform);

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

	cbCamera.farClip = mProjectionInfo.mFarClip;

	mDeviceContext->UpdateSubresource(mViewProjectionBuffer, 0, nullptr,
		&cbCamera, 0, 0);

	mViewProjectionNeedsUpdate = false;
}

void bsCamera::rotateAboutAxis(const hkVector4& axis, float degrees)
{
	float radians = XMConvertToRadians(degrees);

	hkRotation rot;
	rot.setAxisAngle(axis, radians);
	mTransform.getRotation().mul(rot);

	hkTransform rotatedTransform(mTransform);
	rotatedTransform.getRotation().invert(0.00001f);
	rotatedTransform.getTranslation().mul4(-1.0f);
	
	mPhantom->setTransform(rotatedTransform);
	mRigidBody->setTransform(rotatedTransform);

	mViewNeedsUpdate = true;
}

void bsCamera::translate(float x, float y, float z)
{
	mTransform.getTranslation().add3clobberW(hkVector4(-x, -y, -z));

	hkTransform rotatedTransform(mTransform);
	rotatedTransform.getRotation().invert(0.00001f);
	rotatedTransform.getTranslation().mul4(-1.0f);

	mPhantom->setTransform(rotatedTransform);
	mRigidBody->setTransform(rotatedTransform);
	
	mViewNeedsUpdate = true;
}

void bsCamera::translateRelative(float x, float y, float z)
{
	//TODO: Fix or remove this function
	assert(false);

	hkVector4 translation(x, y, z);
	//mTransform.getRotation().multiplyVector(translation, translation);
	
	translation.setMul3(mTransform.getRotation(), translation);
	mTransform.getTranslation().add3clobberW(translation);


	/*
	hkVector4 newPosition(x, y, -z);
	hkTransform trns = mPhantom->getTransform();
	trns.getTranslation().setTransformedPos(trns, newPosition);
	
	mPhantom->setTransform(trns);
	mRigidBody->setTransform(trns);
	*/
	/*
	hkVector4 newTranslation(0.0f, 0.0f, 0.0f);// = transform.getTranslation();
	//newTranslation.setTransformedPos(transform, hkVector4(x, y, z));
	newTranslation.setMul3(mTransform.getRotation(), hkVector4(-x, -y, -z));

	
	mTransform.getTranslation().add3clobberW(newTranslation);
	*/
	//transform.setTranslation(newTranslation);


	/*
	//current.add3clobberW(hkVector4(x, y, z));
	hkVector4 newTrans(x, y, z);
	newTrans.add3clobberW(current);
	//newTrans.setMul3(transform.getRotation(), current);
	newTrans.setTransformedPos(transform, current);

	transform.setTranslation(newTrans);
	*/

	hkTransform rotatedTransform(mTransform);
	rotatedTransform.getRotation().invert(0.00001f);
	rotatedTransform.getTranslation().mul4(-1.0f);

	mPhantom->setTransform(rotatedTransform);
	mRigidBody->setTransform(rotatedTransform);
	

	mViewNeedsUpdate = true;

	return;


	/*
	hkTransform transform = mPhantom->getTransform();
	//transform.getTranslation().sub3clobberW(hkVector4(x, y, z));

	const hkVector4& oldTranslation = transform.getTranslation();
	hkVector4 newTranslation(x, y, z);
	newTranslation.setRotatedDir(transform.getRotation(), newTranslation);
	transform.getTranslation().add3clobberW(newTranslation);

	mPhantom->setTransform(transform);
	mRigidBody->setTransform(transform);

	mViewNeedsUpdate = true;*/
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
