#include "StdAfx.h"

#include "bsPrimitiveCreator.h"

#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/HeightField/Plane/hkpPlaneShape.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "bsEntity.h"
#include "bsMeshCache.h"
#include "bsMath.h"


bsEntity* bsPrimitiveCreator::createSphere(float radius)
{
	bsEntity* entity = new bsEntity();
	entity->attachMesh(mMeshCache.getMesh("sphere_1m_d.bsm"));

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpSphereShape(radius);
	rbCinfo.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	entity->getTransform().setLocalScaleUniform(radius * 2.0f);

	return entity;
}

bsEntity* bsPrimitiveCreator::createBox(const XMVECTOR& halfExtents)
{
	bsEntity* entity = new bsEntity();
	entity->attachMesh(mMeshCache.getMesh("unit_cube.bsm"));

	hkVector4 halfExt(bsMath::toHK(halfExtents));
	//Subtract convex radius from the half extents to prevent the box from
	//appearing to hover over the ground.
	const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
		hkConvexShapeDefaultRadius);
	halfExt.sub(convexRadius);

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpBoxShape(halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	entity->getTransform().setLocalScale(XMVectorScale(halfExtents, 2.0f));

	return entity;
}

bsEntity* bsPrimitiveCreator::createPlane(const XMVECTOR& halfExtents)
{
	bsEntity* entity = new bsEntity();
	entity->attachMesh(mMeshCache.getMesh("plane_1m.bsm"));

	hkVector4 halfExt;
	halfExt.load<4>(halfExtents.m128_f32);

	hkpRigidBodyCinfo rbCinfo;
	//Create plane shape point upwards at origin.
	rbCinfo.m_shape = new hkpPlaneShape(hkVector4::getConstant<HK_QUADREAL_0100>(), hkVector4::getZero(), halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_FIXED;

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	XMVECTOR scale = XMVectorScale(halfExtents, 2.0f);
	//Set Y scale to 1 regardless of input.
	scale = XMVectorSetY(scale, 1.0f);
	entity->getTransform().setLocalScale(scale);

	return entity;
}
