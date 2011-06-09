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
class hkpHybridBroadPhase;


/*	Projection information used to construct the camera.
*/
struct bsProjectionInfo
{
	bsProjectionInfo(float fieldOfViewDegrees, float farClip, float nearClip,
		float aspectRatio)
		: mFieldOfView(fieldOfViewDegrees)
		, mFarClip(farClip)
		, mNearClip(nearClip)
		, mAspectRatio(aspectRatio)
	{}

	//Field of view in degrees
	float	mFieldOfView;
	//Distance to the far clip plane
	float	mFarClip;
	//Distance to the near clip plane
	float	mNearClip;
	//Aspect ratio of view space x:y
	float	mAspectRatio;
};

/*	A camera describing a position in 3D space from which a scene will be rendered.
	
	The camera will automatically upload its transformation to the GPU when it changes.
*/
class bsCamera
{
public:
	bsCamera(const bsProjectionInfo& projectionInfo, bsSceneGraph* sceneGraph,
		bsHavokManager* havokManager);

	~bsCamera();

	/*	Rotates the camera so that it looks at the target position.
	*/
	void lookAt(const hkVector4& targetPosition);

	/*	Rotates the camera around a specified axis.
	*/
	void rotateAboutAxis(const hkVector4& axis, float degrees);

	/*	Translates the camera.
	*/
	void translate(const hkVector4& translation);

	/*	Sets the camera's position in world space.
	*/
	void setPosition(const hkVector4& position);

	/*	Returns the camera's position in world space.
	*/
	inline const hkVector4& getPosition() const
	{
		return mTransform.getTranslation();
	}

	inline const hkTransform& getTransform() const
	{
		return mTransform;
	}

	inline const hkTransform& getTransform2() const
	{
		return mPhantom->getTransform();
	}

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

		mProjectionNeedsUpdate = true;
	}

	/*	Returns a vector of all scene nodes that overlap with the frustum.
	*/
	std::vector<bsSceneNode*> getVisibleSceneNodes() const;

	/*	Updates and uploads transformation matrix to the GPU if it has changed.
	*/
	void update();

	/*	Rotates the camera around the X axis.
	*/
	void rotateX(float angleRadians);

	/*	Rotates the camera around the Y axis.
	*/
	void rotateY(float angleRadians);

private:
	void constructFrustum();

	/*	Updates the view matrix and then updates the view projection matrix.
	*/
	void updateView();

	/*	Updates projection matrix with current projection info, and then updates
		the view projection matrix.
	*/
	void updateProjection();

	/*	Updates view projection matrix based on current projection and view matrices
		and uploads it to the GPU
	*/
	void updateViewProjection();

	/*	Syncronizes the phantom used for visibility detection's transform with the camera
		so that visibility detection will be as accurate as possible.
	*/
	void updatePhantomTransform();



	
	XMFLOAT4X4	mView;
	XMFLOAT4X4	mProjection;
	XMFLOAT4X4	mViewProjection;

	bool		mProjectionNeedsUpdate;
	bool		mViewNeedsUpdate;
	bool		mViewProjectionNeedsUpdate;

	bsProjectionInfo	mProjectionInfo;

	ID3D11Buffer*	mViewProjectionBuffer;


	bsSceneGraph*				mSceneGraph;
	const hkpHybridBroadPhase*	mHybridBroadphase;
	ID3D11DeviceContext*		mDeviceContext;

	bsNodeCollectorPhantom*		mPhantom;
	hkpRigidBody*				mRigidBody;

	hkTransform		mTransform;

	float	mRotationX;
	float	mRotationY;
};

#endif // BS_CAMERA_H
