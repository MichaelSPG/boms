#include "bsEntity.h"

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "bsAssert.h"
#include "bsLight.h"
#include "bsLine3D.h"
#include "bsCamera.h"


/*	This file contains template specializations for attach and detach functions.
	For every valid component, a specialization exists.
	For anything else, a specialization with a static_assert exists.
*/

//////////////////////////////////////////////////////////////////////////
//attach specializations.

//Rigid body
template <>
inline void bsEntity::attach(hkpRigidBody* rigidBody)
{
	BS_ASSERT2(mRigidBody == nullptr, "Trying to attach a rigid body, but a rigid"
		" body is already attached");
	BS_ASSERT2(!rigidBody->hasProperty(BSPK_ENTITY_POINTER), "Trying to attach a"
		" rigid body, but it is still attached to a different entity");

	mRigidBody = rigidBody;
	mRigidBody->addReference();
	mRigidBody->setProperty(BSPK_ENTITY_POINTER, hkpPropertyValue(this));

	syncRigidBodyWithOwner();
}

//Mesh
template <>
inline void bsEntity::attach(const std::shared_ptr<bsMesh>& mesh)
{
	BS_ASSERT2(!mMesh, "Trying to attach a mesh, but a mesh is already attached");

	mMesh = mesh;
}

template <>
inline void bsEntity::attach(std::shared_ptr<bsMesh> mesh)
{
	attach<const std::shared_ptr<bsMesh>&>(mesh);
}

//Light
template <>
inline void bsEntity::attach(bsLight* light)
{
	BS_ASSERT2(mLight == nullptr, "Trying to attach a light, but a light is already attached");

	mLight = light;
}

//Line
template <>
inline void bsEntity::attach(bsLine3D* lineRenderer)
{
	BS_ASSERT2(mLineRenderer == nullptr, "Trying to attach a light, but a light is already attached");

	mLineRenderer = lineRenderer;
}

//Camera
template <>
inline void bsEntity::attach(bsCamera* camera)
{
	BS_ASSERT2(mCamera == nullptr, "Trying to attach a camera, but a camera is already attached");

	mCamera = camera;
	mCamera->setEntity(this);
}

//3D Text
template<>
inline void bsEntity::attach(bsText3D* text3D)
{
	BS_ASSERT2(mText3D == nullptr, "Trying to attach a 3D text, but a 3D text is already attached");

	mText3D = text3D;
}


//Unknown component types.
template <typename T>
inline void bsEntity::attach(T component)
{
	static_assert(false, "Invalid component type");
}




//////////////////////////////////////////////////////////////////////////
//detach specializations.

//Rigid body
template <>
inline void bsEntity::detach<hkpRigidBody*>()
{
	BS_ASSERT2(mRigidBody != nullptr, "Trying to detach a rigid body, but no rigid"
		" body is attached");

	mRigidBody->removeProperty(BSPK_ENTITY_POINTER);
	mRigidBody->removeReference();
	mRigidBody = nullptr;
}

template <>
inline void bsEntity::detach<hkpRigidBody>()
{
	//Forward to pointer version, this is here just in case people forget to type the *.
	detach<hkpRigidBody*>();
}


//Mesh
template <>
inline void bsEntity::detach<std::shared_ptr<bsMesh>>()
{
	BS_ASSERT2(mMesh, "Trying to detach a mesh, but no mesh is attached");

	mMesh.reset();
}

template <>
inline void bsEntity::detach<bsMesh>()
{
	detach<std::shared_ptr<bsMesh>>();
}

template <>
inline void bsEntity::detach<bsMesh*>()
{
	detach<std::shared_ptr<bsMesh>>();
}


//Light
template <>
inline void bsEntity::detach<bsLight*>()
{
	BS_ASSERT2(mLight != nullptr, "Trying to detach a light, but no light is attached");

	delete mLight;
	mLight = nullptr;
}

template <>
inline void bsEntity::detach<bsLight>()
{
	detach<bsLight*>();
}


//Line
template <>
inline void bsEntity::detach<bsLine3D*>()
{
	BS_ASSERT2(mLineRenderer != nullptr, "Trying to detach a line renderer,"
		" but no line renderer is attached");

	delete mLineRenderer;
	mLineRenderer = nullptr;
}

template <>
inline void bsEntity::detach<bsLine3D>()
{
	detach<bsLine3D*>();
}

//Camera
template <>
inline void bsEntity::detach<bsCamera*>()
{
	BS_ASSERT2(mCamera != nullptr, "Trying to detach a camera, but no camera is attached");

	delete mCamera;
	mCamera = nullptr;
}

template <>
inline void bsEntity::detach<bsCamera>()
{
	detach<bsCamera*>();
}

//3D text
template<>
inline void bsEntity::detach<bsText3D>()
{
	mText3D = nullptr;
}

template <typename T>
void bsEntity::detach()
{
	static_assert(false, "Invalid component type");
}




//////////////////////////////////////////////////////////////////////////
//getComponent specializations.


//Rigid body
template <>
inline hkpRigidBody* bsEntity::getComponent()
{
	return mRigidBody;
}

//Mesh
template <>
inline std::shared_ptr<bsMesh>& bsEntity::getComponent()
{
	return mMesh;
}

//Light
template <>
inline bsLight* bsEntity::getComponent()
{
	return mLight;
}

//Line
template <>
inline bsLine3D* bsEntity::getComponent()
{
	return mLineRenderer;
}

//Camera
template <>
inline bsCamera* bsEntity::getComponent()
{
	return mCamera;
}

//3D text
template<>
inline bsText3D* bsEntity::getComponent()
{
	return mText3D;
}

template <typename T>
inline T bsEntity::getComponent()
{
	static_assert(false, "Invalid component type");
}

