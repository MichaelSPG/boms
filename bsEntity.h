#pragma once

#include <memory>//std::shared_ptr
#include <stdlib.h>//_aligned_malloc/_aligned_free

#include <Common/Base/hkBase.h>
#include <Common/Base/Types/hkRefPtr.h>
#include <Physics/Dynamics/World/hkpWorldObject.h>

#include "bsTransform.h"
#include "bsCollision.h"
#include "bsAssert.h"

class hkpRigidBody;

class bsMesh;
class bsLineRenderer;
class bsLight;
class bsCamera;
class bsText3D;
class bsScene;
class bsMeshRenderer;


/*	Keys used for properties attached to Havok rigid bodies and similar, making it
	possible to find an entity from a Havok rigid body.
*/
enum bsPropertyKeys
{
	//Pointer to the bsEntity owning the Havok entity.
	BSPK_ENTITY_POINTER = 10000
};

/*	Utility function to get a bsEntity from a Havok object.
*/
inline bsEntity& bsGetEntity(const hkpWorldObject& havokObject)
{
	BS_ASSERT2(havokObject.hasProperty(BSPK_ENTITY_POINTER), "Object does not have an"
		" entity property");
	return *static_cast<bsEntity*>(havokObject.getProperty(BSPK_ENTITY_POINTER).getPtr());
}


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
	void attachMeshRenderer(bsMeshRenderer& mesh);
	void attachLight(bsLight& light);
	void attachLineRenderer(bsLineRenderer& lineRenderer);
	void attachCamera(bsCamera& camera);
	void attachTextRenderer(bsText3D& textRenderer);

	/*	Detatches a component from this entity. This deletes the component.
	*/
	void detachRigidBody();
	void detachMeshRenderer();
	void detachLight();
	void detachLineRenderer();
	void detachCamera();
	void detachTextRenderer();

	/*	Read-only access to components.
		If the component does not exist, null is returned.
	*/
	const bsTransform& getTransform() const;
	const hkpRigidBody* getRigidBody() const;
	const bsMeshRenderer* getMeshRenderer() const;
	const bsLight* getLight() const;
	const bsLineRenderer* getLineRenderer() const;
	const bsCamera* getCamera() const;
	const bsText3D* getTextRenderer() const;

	/*	Read/write access to components.
		If the component does not exist, null is returned.
	*/
	bsTransform& getTransform();
	hkpRigidBody* getRigidBody();
	bsMeshRenderer* getMeshRenderer();
	bsLight* getLight();
	bsLineRenderer* getLineRenderer();
	bsCamera* getCamera();
	bsText3D* getTextRenderer();

	/*	Computes this entity's local space bounding sphere and returns the result.
		The bounding sphere's center is in local space.
	*/
	bsCollision::Sphere getBoundingSphere() const;


	/*	Returns a pointer to the scene this entity is currently in, or null if it's not
		in a scene.
	*/
	const bsScene* getScene() const;
	bsScene* getScene();

	void addedToScene(bsScene& scene, unsigned int id);
	void removedFromScene(bsScene& scene);

	

	/*	Recalculates the bounding sphere from all attached graphical components.
		This is called when a graphical component has been detatched, or when an attached
		graphical component has been modified.
	*/
	void calculateLocalBoundingSphere();

private:
	/*	Updates own bounding sphere to include a new bounding sphere.
		This function is called after a graphical component has been attached.
	*/
	void updateBoundingSphere(const bsCollision::Sphere& newSphereToInclude);

	//This entity's unique ID for this scene.
	unsigned int	mSceneID;
	bsTransform		mTransform;

	/*	This bounding sphere encapsulates every component's graphical representation
		(assuming they provide correct bounding spheres).
		This sphere does not include any rotation/translation/scale.
	*/
	bsCollision::Sphere		mBoundingSphere;

	bsMeshRenderer*			mMeshRenderer;
	bsLineRenderer*			mLineRenderer;
	bsLight*				mLight;
	bsCamera*				mCamera;
	bsText3D*				mTextRenderer;

	hkRefPtr<hkpRigidBody>	mRigidBody;

	//The scene in which this entity exists, or null if it's not in a scene.
	bsScene*		mScene;
};
