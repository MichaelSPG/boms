#include "Camera.h"

#include "SceneGraph.h"


Camera::Camera(float fieldOfViewDegrees, float farClip, float nearClip, float aspectRatio,
	SceneGraph*	sceneGraph)
	: mLookAt(0.0f, 0.0f, 0.0f)
	, mUp(0.0f, 1.0f, 0.0f)
	, mPosition(0.0f, 0.0f, 0.0f)

	, mFoV(XMConvertToRadians(fieldOfViewDegrees))
	, mFarClip(farClip)
	, mNearClip(nearClip)
	, mAspectRatio(aspectRatio)
	
	, mSceneGraph(sceneGraph)
	, mDeviceContext(mSceneGraph->getRenderer()->getDeviceContext())
	/*
	, mProjection(XMMatrixIdentity())
	, mView(XMMatrixIdentity())
	, mViewProjection(XMMatrixIdentity())
	, mWorld(XMMatrixIdentity())
	*/
{
	
	XMStoreFloat4x4(&mProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mViewProjection, XMMatrixIdentity());
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
	
	/*
	mProjection = XMMatrixIdentity();
	mView = XMMatrixIdentity();
	mViewProjection = XMMatrixIdentity();
	mWorld = XMMatrixIdentity();
	*/

	

	createProjection();

	

	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBChangesEveryFrame);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	HRESULT res = mSceneGraph->getRenderer()->getDevice()->CreateBuffer(&bufferDescription,
		nullptr, &mViewProjectionBuffer);
	assert(SUCCEEDED(res));

	/*
	bufferDescription.ByteWidth = sizeof(CBChangesNever);
	res = mSceneGraph->getRenderer()->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mViewBuffer);
	assert(SUCCEEDED(res));
	*/
	//ID3D11Buffer* p = mSceneGraph->getRenderer()->getChangesNeverBuffer();
	//mDeviceContext->VSSetConstantBuffers(0, 1, &p);




	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	//Constant buffers
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBChangesNever);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	if (!SUCCEEDED(mSceneGraph->getRenderer()->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mViewBuffer)))
		throw std::exception("Failed to create constant buffer");

	mDeviceContext->VSSetConstantBuffers(0, 1, &mViewBuffer);

	//////////////////////////////////////////////////////////////////////////
	/*
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	*/
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -6.0f, 0.0f);
	XMVECTOR At =  XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mPosition.x = XMVectorGetX(Eye);
	mPosition.y = XMVectorGetY(Eye);
	mPosition.z = XMVectorGetZ(Eye);
	mPosition.x += 5.0f;

	/*mView =*/ XMStoreFloat4x4(&mView, XMMatrixLookAtLH(Eye, At, Up));

	Math::setTranslationInFloat4x4(mView, mPosition);

	CBChangesNever cbChangesNever;
	//XMStoreFloat4x4(&cbChangesNever.mView, XMMatrixTranspose(mView));
	XMStoreFloat4x4(&cbChangesNever.mView, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
	//cbChangesNever.mView = XMMatrixTranspose(mView);
	mDeviceContext->UpdateSubresource(mViewBuffer, 0, nullptr, &cbChangesNever, 0, 0);


	update();
}

Camera::~Camera()
{
	
}

inline void Camera::lookAt(const XMFLOAT3& position)
{

}

inline void Camera::setPosition(const XMFLOAT3& translation)
{

}

inline void Camera::setPosition(float x, float y, float z)
{

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