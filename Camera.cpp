#include "Camera.h"

#include <assert.h>

#include "SceneGraph.h"


Camera::Camera(const ProjectionInfo& projectionInfo, SceneGraph* sceneGraph)
	: mLookAt(0.0f, 0.0f, 0.0f)
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
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());

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
		throw std::exception("Failed to create view projection buffer");
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

	updateProjection();
	updateView();
}

Camera::~Camera()
{
	mViewProjectionBuffer->Release();
}

inline void Camera::lookAt(const XMFLOAT3& position)
{
	assert(!"Camera::lookAt not implemented");

	updateView();
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
	updateView();
}

inline void Camera::setPosition(float x, float y, float z)
{
	assert(!"Camera::setPosition not implemented");

	updateView();
}

void Camera::update()
{
	/*
	CBCamera constBuffer;
	XMStoreFloat4x4(&constBuffer.camera, mViewProjection);

	mSceneGraph->getRenderer()->getDeviceContext()->UpdateSubresource(mViewProjectionBuffer, 0, nullptr,
		&constBuffer, 0, 0);
	mSceneGraph->getRenderer()->getDeviceContext()->VSSetConstantBuffers(3, 1, &mViewProjectionBuffer);
	*/
}


