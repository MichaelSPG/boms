#pragma once

#include <memory>//std::shared_ptr
#include <stdlib.h>//_aligned_malloc/_aligned_free

#include <Common/Base/hkBase.h>
#include <Common/Base/Types/hkRefPtr.h>

#include "bsTransform.h"
#include "bsCollision.h"

class hkpRigidBody;

class bsMesh;
class bsLineRenderer;
class bsLight;
class bsCamera;
class bsText3D;
class bsScene;

typedef std::shared_ptr<bsMesh> bsSharedMesh;


/*	Entities are used to represent an object in 3D space which can have multiple components
	attached to it.
	For example, an entity can be used to represent a car by attaching the car mesh,
	an exhaust particle effect and an engine sound to it.

	After creating an entity, add it to a scene to make it active.

	An entity will own all of its components (except for reference counted components),
	and will delete them when the entity is deleted.
*/
__declspec(align(16)) class bsEntity
{
public:
	inline void* operator new(size_t)
	{
		return _aligned_malloc(sizeof(bsEntity), 16);
	}
	inline void operator delete(void* p)
	{
		_aligned_free(p);
	}


	bsEntity();

	~bsEntity();


	/*	Attaches a component to this entity.
	*/
	void attachRigidBody(hkpRigidBody& rigidBody);
	void attachMesh(const bsSharedMesh& mesh);
	void attachLight(bsLight& light);
	void attachLineRenderer(bsLineRenderer& lineRenderer);
	void attachCamera(bsCamera& camera);
	void attachTextRenderer(bsText3D& textRenderer);

	/*	Detatches a component from this entity. This deletes the component.
	*/
	void detachRigidBody();
	void detachMesh();
	void detachLight();
	void detachLineRenderer();
	void detachCamera();
	void detachTextRenderer();

	/*	Read-only access to components.
		If the component does not exist, null is returned.
	*/
	const bsTransform& getTransform() const;
	const hkpRigidBody* getRigidBody() const;
	const bsSharedMesh& getMesh() const;
	const bsLight* getLight() const;
	const bsLineRenderer* getLineRenderer() const;
	const bsCamera* getCamera() const;
	const bsText3D* getTextRenderer() const;

	/*	Read/write access to components.
		If the component does not exist, null is returned.
	*/
	bsTransform& getTransform();
	hkpRigidBody* getRigidBody();
	bsSharedMesh& getMesh();
	bsLight* getLight();
	bsLineRenderer* getLineRenderer();
	bsCamera* getCamera();
	bsText3D* getTextRenderer();


	/*	Returns a pointer to the scene this entity is currently in, or null if it's not
		in a scene.
	*/
	const bsScene* getScene() const;

	void addedToScene(bsScene& scene, unsigned int id);
	void removedFromScene(bsScene& scene);


private:
	//This entity's unique ID for this scene.
	unsigned int	mSceneID;
	bsTransform		mTransform;

	bsCollision::Sphere		mBoundingSphere;

	bsSharedMesh			mMesh;
	bsLineRenderer*			mLineRenderer;
	bsLight*				mLight;
	bsCamera*				mCamera;
	bsText3D*				mTextRenderer;

	hkRefPtr<hkpRigidBody>	mRigidBody;

	//The scene in which this entity exists, or null if it's not in a scene.
	bsScene*		mScene;
};
