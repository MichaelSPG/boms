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
		/*
		mViewProjection = XMMatrixMultiply(mViewProjection,
			XMMatrixTranslationFromVector(XMLoadFloat3(&mPosition)));
		*/
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
		mFoV = XMConvertToRadians(degrees);
		createProjection();
	}

	inline const XMFLOAT4X4& getViewProjection()
	{
		return mViewProjection;
	}

	//Updates the view buffer
	inline void updateView()
	{
		/*
		auto temp = XMLoadFloat4x4(&mView);
		temp = XMMatrixTranspose(temp);
		Math::setTranslationInMatrix(temp, mPosition);
		XMStoreFloat4x4(&mView, temp);
		*/
		Math::setTranslationInFloat4x4(mView, mPosition);

		CBChangesNever cbNever;
		XMStoreFloat4x4(&cbNever.mView, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
		mDeviceContext->UpdateSubresource(mViewBuffer, 0, nullptr, &cbNever, 0, 0);

		


		//XMStoreFloat4x4(&cbNever.mView, XMMatrixTranspose(mView));
		



		//context->UpdateSubresource(mEveryFrame, 0, nullptr, &cb, 0, 0);
	//	mDeviceContext->VSSetConstantBuffers(0, 1, &mViewBuffer);
		//mDeviceContext->PSSetConstantBuffers(0, 1, &mViewBuffer);
	}

private:
	inline void createProjection()
	{
		/*mProjection =*/ XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(mFoV, mAspectRatio, mNearClip, mFarClip));
		//mViewProjection = XMMatrixMultiply(mView, mProjection);
	}

	void update();

	
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mView;
	XMFLOAT4X4	mViewProjection;
	XMFLOAT4X4	mWorld;
	/*
	XMMATRIX	mProjection;
	XMMATRIX	mView;
	XMMATRIX	mViewProjection;
	XMMATRIX	mWorld;
	*/
	XMFLOAT3	mLookAt;
	XMFLOAT3	mUp;
	XMFLOAT3	mPosition;

	float	mFoV;
	float	mFarClip;
	float	mNearClip;
	float	mAspectRatio;

	ID3D11Buffer*	mViewProjectionBuffer;
	ID3D11Buffer*	mViewBuffer;

	SceneGraph*				mSceneGraph;
	ID3D11DeviceContext*	mDeviceContext;
};

#endif // CAMERA_H