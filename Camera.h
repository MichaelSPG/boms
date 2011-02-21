#ifndef CAMERA_H
#define CAMERA_H

#include "Dx11Renderer.h"
#include "SceneGraph.h"

class SceneGraph;

struct ProjectionInfo
{
	ProjectionInfo(float fieldOfViewDegrees, float farClip, float nearClip,
		float aspectRatio)
		: mFieldOfView(fieldOfViewDegrees)
		, mFarClip(farClip)
		, mNearClip(nearClip)
		, mAspectRatio(aspectRatio)

	{}

	float	mFieldOfView;
	float	mFarClip;
	float	mNearClip;
	float	mAspectRatio;
};


class Camera
{
public:
	enum Axis
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z
	};

	/**	
		Aspect ratio of view space X:Y.
	*/
	Camera(const ProjectionInfo& projectionInfo, SceneGraph* sceneGraph);
	~Camera();

	void lookAt(const XMFLOAT3& position);

	void rotateAboutAxis(Axis axis, float degrees)
	{
		switch (axis)
		{
		case AXIS_X:
			XMStoreFloat4x4(&mView, XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationX(degrees)));
			break;

		case AXIS_Y:
			XMStoreFloat4x4(&mView, XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationY(degrees)));
			break;

		case AXIS_Z:
			XMStoreFloat4x4(&mView, XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixRotationZ(degrees)));
			break;
		}
		updateView();
	}

	inline void translate(const XMFLOAT3& translation)
	{
		translate(translation.x, translation.y, translation.z);
	}

	inline void translate(float x, float y, float z)
	{
		mPosition.x += x;
		mPosition.y += y;
		mPosition.z += z;

		update();
	}

	inline void setPosition(const XMFLOAT3& translation);

	inline void setPosition(float x, float y, float z);

	inline const XMFLOAT3& getPosition() const
	{
		return mPosition;
	}
	/*
	inline void setFieldOfView(float degrees)
	{
		mFieldOfView = XMConvertToRadians(degrees);
		//createProjection();
	}*/
	/*
	inline const XMFLOAT4X4& getViewProjection()
	{
		return mViewProjection;
	}*/

	//Updates the view buffer
	inline void updateView()
	{
		Math::setTranslationInFloat4x4(mView, mPosition);

		CBChangesNever cbNever;
		XMStoreFloat4x4(&cbNever.mView, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
		mDeviceContext->UpdateSubresource(mViewBuffer, 0, nullptr, &cbNever, 0, 0);
	}

	inline void createProjection()
	{
		XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(mProjectionInfo.mFieldOfView,
			mProjectionInfo.mAspectRatio, mProjectionInfo.mNearClip, mProjectionInfo.mFarClip));

		CBChangesOnResize cbChangesOnResize;
		cbChangesOnResize.mProjection = Math::XMFloat4x4Transpose(mProjection);

		mDeviceContext->UpdateSubresource(mProjectionBuffer, 0, nullptr, &cbChangesOnResize, 0, 0);
	}

	inline const ProjectionInfo& getProjectionInfo()
	{
		return mProjectionInfo;
	}

	inline void setProjectionInfo(const ProjectionInfo& projectionInfo)
	{
		mProjectionInfo = projectionInfo;

		createProjection();
	}

private:
	

	void update();

	
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mView;
//	XMFLOAT4X4	mViewProjection;
	XMFLOAT4X4	mWorld;

	XMFLOAT3	mLookAt;
	XMFLOAT3	mUp;
	XMFLOAT3	mPosition;

	ProjectionInfo	mProjectionInfo;

//	ID3D11Buffer*	mViewProjectionBuffer;
	ID3D11Buffer*	mViewBuffer;
	ID3D11Buffer*	mProjectionBuffer;

	SceneGraph*				mSceneGraph;
	ID3D11DeviceContext*	mDeviceContext;
};

#endif // CAMERA_H