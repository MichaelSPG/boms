#pragma once

#include <D3DX11.h>
#include <Windows.h>
#include <xnamath.h>

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

#include "bsScene.h"

class bsScene;
class bsEntity;
class bsCamera;
class bsDx11Renderer;

void* allocateCamera();
void deallocateCamera(void* p);


/*	Projection information used to construct the camera.
*/
struct bsProjectionInfo
{
	bsProjectionInfo(float fieldOfViewDegrees, float farClip, float nearClip,
		float aspectRatio, float screenWidth, float screenHeight)
		: mFieldOfView(fieldOfViewDegrees)
		, mFarClip(farClip)
		, mNearClip(nearClip)
		, mAspectRatio(aspectRatio)
		, mScreenSize(screenWidth, screenHeight)
	{}

	//Field of view in degrees
	float	mFieldOfView;
	//Distance to the far clip plane
	float	mFarClip;
	//Distance to the near clip plane
	float	mNearClip;
	//Aspect ratio of view space x:y
	float	mAspectRatio;

	//Size of screen in pixels, x=width, y=height.
	XMFLOAT2	mScreenSize;
};

/*	A camera describing a position in 3D space from which a scene will be rendered.
	
	The camera will automatically upload its transformation to the GPU when it changes.
*/
__declspec(align(16)) class bsCamera
{
public:
	void* operator new(size_t)
	{
		return allocateCamera();
	}

	void operator delete(void* p)
	{
		deallocateCamera(p);
	}


	bsCamera(const bsProjectionInfo& projectionInfo, bsDx11Renderer* dx11Renderer);

	~bsCamera();

	
	/*	Returns a reference to the currently active projection info.
		This may not be identical to the projection info the camera was created with.
	*/
	inline const bsProjectionInfo& getProjectionInfo() const
	{
		return mProjectionInfo;
	}

	/*	Sets a new projection info.
		The camera's projection matrix will be recreated and uploaded to the GPU
		after this has been called.
	*/
	inline void setProjectionInfo(const bsProjectionInfo& projectionInfo)
	{
		mProjectionInfo = projectionInfo;

		updateProjection();
	}

	/*	Returns the camera's view matrix.
		This matrix is generated every time this function is called, so storing it if it
		is being used multiple times per frame may save some CPU.
	*/
	const XMMATRIX getView() const;

	inline const XMMATRIX getProjection() const
	{
		return mProjection;
	}

	/*	Returns a vector of all scene nodes that overlap with the frustum.
	*/
	std::vector<bsSceneNode*> getVisibleSceneNodes() const;

	/*	Updates and uploads view/projection matrices to the GPU.
		This should be called at least every time the camera's transform/projection has
		changed.
	*/
	void update();

	inline bsEntity* getEntity() const
	{
		return mEntity;
	}

	/*	Internal function, called when the camera is attached to a node.
		Calling this function under any other circumstance results in undefined behavior.
	*/
	inline void setEntity(bsEntity* entity)
	{
		mEntity = entity;
	}

	inline bsScene* getScene() const
	{
		return mScene;
	}

	/*	Internal function, called when the camera is added to a scene.
		Calling this function under any other circumstance results in undefined behavior.
	*/
	inline void setScene(bsScene* scene)
	{
		mScene = scene;
	}

	hkpWorldRayCastOutput screenPointToWorldRay(const XMVECTOR& screenPoint,
		float rayLength, XMVECTOR& destinationOut, XMVECTOR& originOut) const;

private:
	/*	Updates projection matrix with current projection info. Called whenever projection
		info has been modified.
	*/
	void updateProjection();

	/*	Updates view projection matrix based on current projection and view matrices
		and uploads it to the GPU
	*/
	void updateViewProjection();


	XMMATRIX	mProjection;

	bsProjectionInfo	mProjectionInfo;

	ID3D11Buffer*	mCameraConstantBuffer;

	/*	The entiy this camera is attached to, or null if not attached to anything.
	*/
	bsEntity*	mEntity;
	bsScene*	mScene;

	ID3D11DeviceContext*	mDeviceContext;
};


inline void* allocateCamera()
{
	return _aligned_malloc(sizeof(bsCamera), 16);
}

inline void deallocateCamera(void* ptr)
{
	_aligned_free(ptr);
}
