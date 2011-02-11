#ifndef CAMERA_H
#define CAMERA_H

#include "Dx11Renderer.h"
#include "SceneGraph.h"

class SceneGraph;

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
	Camera(float fieldOfViewDegrees, float farClip, float nearClip, float aspectRatio,
		SceneGraph*	sceneGraph);
	~Camera();

	void lookAt(const XMFLOAT3& position);

	void rotateAboutAxis(Axis axis, float degrees)
	{
		/*
		switch (axis)
		{
		case AXIS_X:
			mViewProjection = XMMatrixMultiply(mViewProjection, XMMatrixRotationX(degrees));
			break;

		case AXIS_Y:
			mViewProjection = XMMatrixMultiply(mViewProjection, XMMatrixRotationY(degrees));
			break;

		case AXIS_Z:
			mViewProjection = XMMatrixMultiply(mViewProjection, XMMatrixRotationZ(degrees));
			break;
		}
		*/
		update();
	}

	inline void translate(const XMFLOAT3& translation)
	{
		translate(translation.x, translation.y, translation.z);
	}

	 void translate(float x, float y, float z)
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

	inline void setFieldOfView(float degrees)
	{
		mFieldOfView = XMConvertToRadians(degrees);
		//createProjection();
	}

	inline const XMFLOAT4X4& getViewProjection()
	{
		return mViewProjection;
	}

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
		XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(mFieldOfView, mAspectRatio, mNearClip, mFarClip));

		CBChangesOnResize cbChangesOnResize;
		XMStoreFloat4x4(&cbChangesOnResize.mProjection, XMMatrixTranspose(XMLoadFloat4x4(&mProjection)));
		//cbChangesOnResize.mProjection = XMMatrixTranspose(mProjection);
		cbChangesOnResize.mProjection = Math::XMFloat4x4Transpose(mProjection);

		mDeviceContext->UpdateSubresource(mProjectionBuffer, 0, nullptr, &cbChangesOnResize, 0, 0);
	}

	inline void updateProjectionParameters(float renderWindowHeight, float renderWindowWidth,
		float fieldOfViewDegrees, float farClip, float nearClip, bool recreateProjection = true)
	{
		mAspectRatio = renderWindowWidth / renderWindowHeight;
		mFieldOfView = XMConvertToRadians(fieldOfViewDegrees);
		mFarClip = farClip;
		mNearClip = nearClip;

		if (recreateProjection)
		{
			createProjection();
		}
	}

private:
	

	void update();

	
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mView;
	XMFLOAT4X4	mViewProjection;
	XMFLOAT4X4	mWorld;

	XMFLOAT3	mLookAt;
	XMFLOAT3	mUp;
	XMFLOAT3	mPosition;

	float	mFieldOfView;
	float	mFarClip;
	float	mNearClip;
	float	mAspectRatio;

	ID3D11Buffer*	mViewProjectionBuffer;
	ID3D11Buffer*	mViewBuffer;
	ID3D11Buffer*	mProjectionBuffer;

	SceneGraph*				mSceneGraph;
	ID3D11DeviceContext*	mDeviceContext;
};

#endif // CAMERA_H