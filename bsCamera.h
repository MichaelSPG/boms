#ifndef BS_CAMERA_H
#define BS_CAMERA_H

#include "bsConfig.h"

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Phantom/hkpCachingShapePhantom.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Common/Base/Types/Geometry/hkStridedVertices.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include <D3DX11.h>
#include <Windows.h>
#include <xnamath.h>

#include "bsSceneGraph.h"
#include "bsNodeCollectorPhantom.h"

class bsSceneGraph;
class bsHavokManager;


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
	bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph,
		bsHavokManager* havokManager);

	~bsCamera();

	inline void lookAt(const XMFLOAT3& position);

	void rotateAboutAxis(const hkVector4& axis, float degrees);

	inline void translate(const XMFLOAT3& translation)
	{
		translate(translation.x, translation.y, translation.z);
	}

	void translate(float x, float y, float z);

	inline void translateRelative(const XMFLOAT3& translation)
	{
		translateRelative(translation.x, translation.y, translation.z);
	}

	void translateRelative(float x, float y, float z);

	inline void setPosition(const XMFLOAT3& translation)
	{
		setPosition(translation.x, translation.y, translation.z);
	}

	inline void setPosition(float x, float y, float z);

	inline const bsProjectionInfo& getProjectionInfo() const
	{
		return mProjectionInfo;
	}

	inline void setProjectionInfo(const bsProjectionInfo& projectionInfo)
	{
		mProjectionInfo = projectionInfo;

		mProjectionNeedsUpdate = true;
	}

	inline std::vector<bsSceneNode*> getVisibleSceneNodes() const
	{
		return mPhantom->getOverlappingSceneNodes();
	}

	//Updates and uploads transformation matrix to the GPU if it has changed.
	void update();

	inline const XMFLOAT4X4& getViewProjectionMatrix()
	{
		update();

		return mViewProjection;
	}

	void rotateX(float angleRadians);

	void rotateY(float angleRadians);

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



	
	XMFLOAT4X4	mView;
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mViewProjection;

	bool		mProjectionNeedsUpdate;
	bool		mViewNeedsUpdate;
	bool		mViewProjectionNeedsUpdate;

	bsProjectionInfo	mProjectionInfo;

	ID3D11Buffer*	mViewBuffer;
	ID3D11Buffer*	mProjectionBuffer;
	ID3D11Buffer*	mViewProjectionBuffer;


	bsSceneGraph*			mSceneGraph;
	ID3D11DeviceContext*	mDeviceContext;

	bsNodeCollectorPhantom*		mPhantom;
	hkpRigidBody*				mRigidBody;

	hkTransform		mTransform;

	float	mRotationX;
	float	mRotationY;
};

#endif // BS_CAMERA_H
