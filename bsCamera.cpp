#include "StdAfx.h"

#include "bsCamera.h"

#include <memory>

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

#include "bsMath.h"
#include "bsEntity.h"
#include "bsScene.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsDx11Renderer.h"
#include "bsConstantBuffers.h"
#include "bsRayCastUtil.h"


bsCamera::bsCamera(const bsProjectionInfo& projectionInfo, bsDx11Renderer* dx11Renderer)
	: mProjectionInfo(projectionInfo)
	, mScene(nullptr)
	, mEntity(nullptr)
	, mDeviceContext(dx11Renderer->getDeviceContext())
{
	//Create view projection buffer
	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBCamera);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	if (FAILED(dx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mCameraConstantBuffer)))
	{
		BS_ASSERT(!"Failed to create view projection buffer");
	}
	mDeviceContext->PSSetConstantBuffers(0, 1, &mCameraConstantBuffer);
	mDeviceContext->VSSetConstantBuffers(0, 1, &mCameraConstantBuffer);

#ifdef BS_DEBUG
	const char* debugMessage = "bsCamera constant buffer";
	mCameraConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(debugMessage),
		debugMessage);
#endif

	updateProjection();
}

bsCamera::~bsCamera()
{
	mCameraConstantBuffer->Release();
}

void bsCamera::update()
{
	BS_ASSERT2(mEntity != nullptr, "Camera must be attached to an entity before calling"
		" update");

	updateViewProjection();
}

void bsCamera::updateProjection()
{
	//Create a projection matrix based on the current projection info.

	mProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(mProjectionInfo.mFieldOfView),
		mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip);

	mFrustum = bsComputeFrustumFromProjection(mProjection);
}

void bsCamera::updateViewProjection()
{
	/*	Calculating view and viewProjection matrix every frame, even if the camera does
		not move since the camera will likely move almost every frame during a game,
		and to avoid needless branching.
	*/
	/*
	const XMVECTOR& pos = mEntity->getTransform().getPosition();
	const XMVECTOR& rot = mEntity->getTransform().getRotation();

	//Calculate view matrix.
	const XMVECTOR inversePosition = XMVector3Rotate(XMVectorScale(pos, -1.0f), rot);

	const XMMATRIX inversePositionMat = XMMatrixTranslationFromVector(inversePosition);
	const XMMATRIX rotationMat = XMMatrixRotationQuaternion(rot);

	const XMMATRIX viewTransform = XMMatrixMultiply(rotationMat, inversePositionMat);
	*/
	const XMMATRIX viewTransform = getViewMatrix();

	XMMATRIX viewProjection = XMMatrixMultiply(viewTransform, mProjection);
	mViewProjection = viewProjection;

	CBCamera cbCam;
	cbCam.view = XMMatrixTranspose(viewTransform);
	cbCam.projection = XMMatrixTranspose(mProjection);
	cbCam.viewProjection = XMMatrixTranspose(viewProjection);
	cbCam.cameraPosition = mEntity->getTransform().getPosition();
	XMVECTOR determinant;
	cbCam.inverseViewProjection = XMMatrixInverse(&determinant, viewProjection);

	mDeviceContext->UpdateSubresource(mCameraConstantBuffer, 0, nullptr, &cbCam, 0, 0);
}

XMMATRIX bsCamera::getViewMatrix() const
{
	const bsTransform& entityTransform = mEntity->getTransform();

	const XMVECTOR& entityPosition = entityTransform.getPosition();
	const XMVECTOR& entityRotation = entityTransform.getRotation();
	const XMVECTOR& entityScale = entityTransform.getScale();

	return XMMatrixAffineTransformation(
		XMVectorDivide(XMVectorReplicate(1.0f), entityScale),//Scale.
		entityPosition,//Rotation origin.
		XMQuaternionInverse(entityRotation),//Rotation.
		XMVectorScale(entityPosition, -1.0f));//Translation.
}

hkpWorldRayCastOutput bsCamera::screenPointToWorldRay(const XMFLOAT2& screenPoint,
	float rayLength, XMVECTOR* destinationOut, XMVECTOR* originOut) const
{
	const XMVECTOR& cameraPosition = mEntity->getTransform().getPosition();
	
	const XMMATRIX view = getViewMatrix();

	//Convert 2D screen point to 3D object space point.
	const XMVECTOR screenPointObjectSpace = bsRayCastUtil::screenSpaceToObjectSpace(
		XMVectorSet(screenPoint.x, screenPoint.y, 0.0f, 1.0f),
		mProjectionInfo.mScreenSize, mProjection, 
		view);
	
	XMVECTOR rayDirection = XMVectorSubtract(screenPointObjectSpace, cameraPosition);
	rayDirection = XMVector3Normalize(rayDirection);
		
	XMVECTOR rayDestination;
	hkpWorldRayCastOutput output;
	bsRayCastUtil::castRay(screenPointObjectSpace, rayDirection, rayLength,
		mScene->getPhysicsWorld(), output, rayDestination);

	//User may not be interested in these two, so check if they're ignored before assigning.
	if (originOut != nullptr)
	{
		*originOut = screenPointObjectSpace;
	}
	if (destinationOut != nullptr)
	{
		*destinationOut = rayDestination;
	}
	return output;
}

const bsFrustum& bsCamera::getFrustum() const
{
	return mFrustum;
}

bsFrustum bsCamera::getTransformedFrustum() const
{
	BS_ASSERT2(mEntity != nullptr, "Camera must be attached to an entity before calling"
		" getTransformedFrustum");

	//Transform local space frustum by the entity the camera is attached to's rotation
	//and position.
	return bsTransformFrustum(mFrustum,
		mEntity->getTransform().getRotation(),
		mEntity->getTransform().getPosition());
}
