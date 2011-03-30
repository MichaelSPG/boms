#ifndef CAMERA_H
#define CAMERA_H

#include "bsDx11Renderer.h"
#include "bsSceneGraph.h"

class bsSceneGraph;

struct bsProjectionInfo
{
	bsProjectionInfo(float fieldOfViewDegrees, float farClip, float nearClip,
		float aspectRatio)
		: mFieldOfView(fieldOfViewDegrees)
		, mFarClip(farClip)
		, mNearClip(nearClip)
		, mAspectRatio(aspectRatio)
	{}

	float	mFieldOfView;
	float	mFarClip;
	float	mNearClip;
	float	mAspectRatio;	//Aspect ratio of view space x:y
};


class bsCamera
{
public:
	enum Axis
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z
	};

	bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph);

	~bsCamera();

	inline void lookAt(const XMFLOAT3& position);

	void rotateAboutAxis(Axis axis, float degrees);

	inline void translate(const XMFLOAT3& translation)
	{
		translate(translation.x, translation.y, translation.z);
	}

	inline void translate(float x, float y, float z)
	{
		mPosition.x += x;
		mPosition.y += y;
		mPosition.z += z;

		mViewNeedsUpdate = true;
	}

	inline void setPosition(const XMFLOAT3& translation)
	{
		setPosition(translation.x, translation.y, translation.z);
	}

	inline void setPosition(float x, float y, float z);

	inline const XMFLOAT3& getPosition() const
	{
		return mPosition;
	}

	inline const bsProjectionInfo& getProjectionInfo() const
	{
		return mProjectionInfo;
	}

	inline void setProjectionInfo(const bsProjectionInfo& projectionInfo)
	{
		mProjectionInfo = projectionInfo;

		mProjectionNeedsUpdate = true;
	}

	//Updates and uploads transformation matrix to the GPU if it has changed.
	void update();

private:
	//Updates the view matrix and then updates the view projection matrix.
	void updateView();

	/*	Updates projection matrix with current projection info, and then updates
		the view projection matrix.
	*/
	void updateProjection();

	/*	Updates view projection matrix based on current projection and view matrices
		and uploads it to the GPU
	*/
	void updateViewProjection();



	
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mView;
	XMFLOAT4X4	mViewProjection;

	bool		mProjectionNeedsUpdate;
	bool		mViewNeedsUpdate;
	bool		mViewProjectionNeedsUpdate;


	XMFLOAT3	mLookAt;
	XMFLOAT3	mUp;
	XMFLOAT3	mPosition;

	bsProjectionInfo	mProjectionInfo;

	ID3D11Buffer*	mViewProjectionBuffer;

	bsSceneGraph*				mSceneGraph;
	ID3D11DeviceContext*	mDeviceContext;
};

#endif // CAMERA_H