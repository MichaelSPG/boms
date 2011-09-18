#include "StdAfx.h"

#include "bsCamera.h"

#include <memory>

#include "bsMath.h"
#include "bsScene.h"
#include "bsLog.h"
#include "bsAssert.h"
#include "bsHavokManager.h"
#include "bsDx11Renderer.h"
#include "bsConstantBuffers.h"
#include "bsSceneNode.h"
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
	mProjection = XMMatrixPerspectiveFovLH(mProjectionInfo.mFieldOfView,
		mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip);
}

void bsCamera::updateViewProjection()
{
	/*	Calculating view and viewProjection matrix every frame, even if the camera does
		not move since the camera will likely move almost every frame during a game,
		and to avoid needless branching.
	*/

	bsSceneNode* owner = mEntity->getOwner();
	const XMVECTOR& pos = owner->getPosition();
	const XMVECTOR& rot = owner->getRotation();

	//Calculate view matrix.
	const XMVECTOR inversePosition = XMVector3Rotate(XMVectorScale(pos, -1.0f), rot);

	const XMMATRIX inversePositionMat = XMMatrixTranslationFromVector(inversePosition);
	const XMMATRIX rotationMat = XMMatrixRotationQuaternion(rot);

	const XMMATRIX viewTransform = XMMatrixMultiply(rotationMat, inversePositionMat);

	XMMATRIX viewProjection = XMMatrixMultiply(viewTransform, mProjection);

	CBCamera cbCam;
	cbCam.view = XMMatrixTranspose(viewTransform);
	cbCam.projection = XMMatrixTranspose(mProjection);
	cbCam.viewProjection = XMMatrixTranspose(viewProjection);
	cbCam.cameraPosition = owner->getPosition();
	XMVECTOR determinant;
	cbCam.inverseViewProjection = XMMatrixInverse(&determinant, viewProjection);

	mDeviceContext->UpdateSubresource(mCameraConstantBuffer, 0, nullptr, &cbCam, 0, 0);
}

std::vector<bsSceneNode*> bsCamera::getVisibleSceneNodes() const
{
	BS_ASSERT2(mScene != nullptr, "Trying to get a camera's visible nodes, but no camera"
		" is not in a scene");

	//Temporary solution, no culling.
	return mScene->getSceneNodes();
}

const XMMATRIX bsCamera::getView() const
{
	XMVECTOR determinant;
	return XMMatrixInverse(&determinant, mEntity->getOwner()->getTransform());
}

hkpWorldRayCastOutput bsCamera::screenPointToWorldRay(const XMVECTOR& screenPoint,
	float rayLength, XMVECTOR& destinationOut, XMVECTOR& originOut) const
{
	XMVECTOR determinant;
	const XMVECTOR screenPointObjectSpace = bsRayCastUtil::screenSpaceToObjectSpace(screenPoint,
		mProjectionInfo.mScreenSize, mProjection, XMMatrixInverse(&determinant, getView()));
		//XMMatrixInverse(&determinant, mView));

	const XMVECTOR& cameraPosition = mEntity->getOwner()->getPosition();
	XMVECTOR dir = XMVectorSubtract(screenPointObjectSpace, cameraPosition);
	dir = XMVector3Normalize(dir);

	const XMMATRIX lookTowardsPoint = XMMatrixLookToLH(cameraPosition,
		XMVectorAdd(cameraPosition, dir), g_XMIdentityR1);

	const XMVECTOR rotationToPoint = XMQuaternionRotationMatrix(lookTowardsPoint);

	hkpWorldRayCastOutput output;
	bsRayCastUtil::castRay(screenPointObjectSpace, rotationToPoint, rayLength,
		mScene->getPhysicsWorld(), output, destinationOut);

	originOut = screenPointObjectSpace;

	return output;
}
