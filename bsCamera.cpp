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

	updateProjection();
}

bsCamera::~bsCamera()
{
	mCameraConstantBuffer->Release();
}

void bsCamera::update()
{
	updateViewProjection();
}

void bsCamera::updateProjection()
{
	//Create a projection matrix based on the current projection info.

	mProjection = XMMatrixPerspectiveFovLH(mProjectionInfo.mFieldOfView,
		mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip);

	mFrustum = bsComputeFrustumFromProjection(mProjection);
}

void bsCamera::updateViewProjection()
{
	/*	Calculating view and viewProjection matrix every frame, even if the camera does
		not move since the camera will likely move almost every frame during a game,
		and to avoid needless branching.
	*/

	const XMVECTOR& pos = mEntity->mTransform.getPosition();
	const XMVECTOR& rot = mEntity->mTransform.getRotation();

	//Calculate view matrix.
	const XMVECTOR inversePosition = XMVector3Rotate(XMVectorScale(pos, -1.0f), rot);

	const XMMATRIX inversePositionMat = XMMatrixTranslationFromVector(inversePosition);
	const XMMATRIX rotationMat = XMMatrixRotationQuaternion(rot);

	const XMMATRIX viewTransform = XMMatrixMultiply(rotationMat, inversePositionMat);

	XMMATRIX viewProjection = XMMatrixMultiply(viewTransform, mProjection);
	mViewProjection = viewProjection;

	CBCamera cbCam;
	cbCam.view = XMMatrixTranspose(viewTransform);
	cbCam.projection = XMMatrixTranspose(mProjection);
	cbCam.viewProjection = XMMatrixTranspose(viewProjection);
	cbCam.cameraPosition = mEntity->mTransform.getPosition();
	XMVECTOR determinant;
	cbCam.inverseViewProjection = XMMatrixInverse(&determinant, viewProjection);

	mDeviceContext->UpdateSubresource(mCameraConstantBuffer, 0, nullptr, &cbCam, 0, 0);
}

std::vector<bsEntity*> bsCamera::getVisibleEntities() const
{
	BS_ASSERT2(mScene != nullptr, "Trying to get a camera's visible nodes, but camera"
		" is not in a scene");

	//Temporary solution, no culling.
	return mScene->getEntities();
}

XMMATRIX bsCamera::getView() const
{
	XMVECTOR determinant;
	return XMMatrixInverse(&determinant, mEntity->mTransform.getTransform());
}

hkpWorldRayCastOutput bsCamera::screenPointToWorldRay(const XMFLOAT2& screenPoint,
	float rayLength, XMVECTOR& destinationOut, XMVECTOR& originOut) const
{
	//Build view matrix with inverted rotation.
	const XMVECTOR& cameraPosition = mEntity->mTransform.getPosition();
	const XMVECTOR cameraRotation = mEntity->mTransform.getRotation();
	const XMMATRIX positionMat = XMMatrixTranslationFromVector(
		XMVectorSubtract(XMVectorZero(), cameraPosition));
	const XMMATRIX rotationMat = XMMatrixRotationQuaternion(cameraRotation);

	const XMMATRIX view = XMMatrixMultiply(positionMat, rotationMat);

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

	originOut = screenPointObjectSpace;
	destinationOut = rayDestination;
	return output;
}
