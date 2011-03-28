#include "Camera.h"

#include <assert.h>

#include "SceneGraph.h"


Camera::Camera(const ProjectionInfo& projectionInfo, SceneGraph* sceneGraph)
	: mProjectionNeedsUpdate(true)
	, mViewNeedsUpdate(true)
	, mViewProjectionNeedsUpdate(true)

	, mLookAt(0.0f, 0.0f, 0.0f)
	, mUp(0.0f, 1.0f, 0.0f)
	, mPosition(0.0f, 0.0f, 0.0f)
	
	, mProjectionInfo(projectionInfo)

	, mSceneGraph(sceneGraph)
	, mDeviceContext(mSceneGraph->getRenderer()->getDeviceContext())
{
	assert(sceneGraph);

	XMStoreFloat4x4(&mProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mViewProjection, XMMatrixIdentity());

	ID3D11Device* device = mSceneGraph->getRenderer()->getDevice();

	//Create view projection buffer
	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBViewProjection);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0u;

	if (FAILED(device->CreateBuffer(&bufferDescription, nullptr, &mViewProjectionBuffer)))
	{
		Log::logMessage("Camera::Camera: failed to create view projection buffer",
			pantheios::SEV_CRITICAL);
		assert(!"Camera::Camera: failed to create view projection buffer");
	}
	mDeviceContext->VSSetConstantBuffers(0u, 1, &mViewProjectionBuffer);

	
	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -6.0f, 0.0f);
	XMVECTOR at =  XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//	mPosition.x = XMVectorGetX(eye);
//	mPosition.y = XMVectorGetY(eye);
//	mPosition.z = XMVectorGetZ(eye);
	/*
	mPosition.x = eye.m128_f32[0];
	mPosition.y = eye.m128_f32[1];
	mPosition.z = eye.m128_f32[2];
	*/

	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(eye, at, up));

	XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(mProjectionInfo.mFieldOfView,
		mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip));
}

Camera::~Camera()
{
	mViewProjectionBuffer->Release();
}

inline void Camera::lookAt(const XMFLOAT3& position)
{
	assert(!"Camera::lookAt not implemented");

	mViewNeedsUpdate = true;
}

void Camera::rotateAboutAxis(Axis axis, float degrees)
{
	switch (axis)
	{
	case AXIS_X:
		XMStoreFloat4x4(&mView,
			XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationX(degrees)));
		break;

	case AXIS_Y:
		XMStoreFloat4x4(&mView,
			XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationY(degrees)));
		break;

	case AXIS_Z:
		XMStoreFloat4x4(&mView,
			XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationZ(degrees)));
		break;
	}
	mViewNeedsUpdate = true;
}

inline void Camera::setPosition(float x, float y, float z)
{
	assert(!"Camera::setPosition not implemented");

	mViewNeedsUpdate = true;
}

void Camera::update()
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

	/*
	CBCamera constBuffer;
	XMStoreFloat4x4(&constBuffer.camera, mViewProjection);

	mSceneGraph->getRenderer()->getDeviceContext()->UpdateSubresource(mViewProjectionBuffer, 0, nullptr,
		&constBuffer, 0, 0);
	mSceneGraph->getRenderer()->getDeviceContext()->VSSetConstantBuffers(3, 1, &mViewProjectionBuffer);
	*/
}

void Camera::updateView()
{
	bsMath::XMFloat4x4SetTranslation(mView, mPosition);

	mViewNeedsUpdate = false;
	//View projection matrix needs to be rebuilt.
	mViewProjectionNeedsUpdate = true;
}

void Camera::updateProjection()
{
	bsMath::XMFloat4x4PerspectiveFovLH(mProjectionInfo.mFieldOfView, mProjectionInfo.mAspectRatio,
		mProjectionInfo.mNearClip, mProjectionInfo.mFarClip, mProjection);

	mProjectionNeedsUpdate = false;
	//View projection matrix needs to be rebuilt.
	mViewProjectionNeedsUpdate = true;
}

void Camera::updateViewProjection()
{
	bsMath::XMFloat4x4Multiply(mView, mProjection, mViewProjection);
	bsMath::XMFloat4x4Transpose(mViewProjection);

	CBViewProjection viewProjection;
	viewProjection.viewProjection = mViewProjection;
	mDeviceContext->UpdateSubresource(mViewProjectionBuffer, 0, nullptr,
		&viewProjection, 0, 0);

	mViewProjectionNeedsUpdate = false;
}

