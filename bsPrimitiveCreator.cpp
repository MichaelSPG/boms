#include "StdAfx.h"

#include "bsPrimitiveCreator.h"

#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/hkpShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/HeightField/Plane/hkpPlaneShape.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
#include <Physics/Collide/Shape/Convex/Cylinder/hkpCylinderShape.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "bsEntity.h"
#include "bsMeshCache.h"
#include "bsTextureCache.h"
#include "bsMath.h"
#include "bsMeshRenderer.h"
#include "bsAssert.h"


bsEntity* bsPrimitiveCreator::createSphere(float radius) const
{
	BS_ASSERT2(radius > 0.0f, "A sphere's radius must be positive and non-zero");

	bsEntity* entity = new bsEntity();
	entity->attachMeshRenderer(*new bsMeshRenderer(mMeshCache.getMesh("sphere_1m_d.bsm"),
		mTextureCache.getDefaultTexture()));

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpSphereShape(radius);
	rbCinfo.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;

	//Compute sphere's mass properties.
	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(rbCinfo.m_shape,
		rbCinfo.m_mass, massProperties);
	rbCinfo.setMassProperties(massProperties);

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	entity->getTransform().setLocalScaleUniform(radius * 2.0f);

	return entity;
}

bsEntity* bsPrimitiveCreator::createBox(const XMVECTOR& halfExtents) const
{
	BS_ASSERT2(XMVectorGetX(halfExtents) > 0.0f, "A box' extents must be positive in every"
		" direction");
	BS_ASSERT2(XMVectorGetY(halfExtents) > 0.0f, "A box' extents must be positive in every"
		" direction");
	BS_ASSERT2(XMVectorGetZ(halfExtents) > 0.0f, "A box' extents must be positive in every"
		" direction");

	bsEntity* entity = new bsEntity();
	entity->attachMeshRenderer(*new bsMeshRenderer(mMeshCache.getMesh("unit_cube.bsm"),
		mTextureCache.getDefaultTexture()));

	hkVector4 halfExt(bsMath::toHK(halfExtents));
	//Subtract convex radius from the half extents to prevent the box from
	//appearing to hover over the ground.
	const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
		hkConvexShapeDefaultRadius);
	halfExt.sub(convexRadius);

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpBoxShape(halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;

	//Compute box' mass properties.
	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(rbCinfo.m_shape,
		rbCinfo.m_mass, massProperties);
	massProperties.scaleToDensity(1.0f);
	rbCinfo.setMassProperties(massProperties);

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	entity->getTransform().setLocalScale(XMVectorScale(halfExtents, 2.0f));

	return entity;
}

bsEntity* bsPrimitiveCreator::createPlane(const XMVECTOR& halfExtents) const
{
	BS_ASSERT2(XMVectorGetX(halfExtents) > 0.0f, "A plane's extents must be positive in"
		" both X and Z");
	BS_ASSERT2(XMVectorGetZ(halfExtents) > 0.0f, "A plane's extents must be positive in"
		" both X and Z");

	bsEntity* entity = new bsEntity();
	entity->attachMeshRenderer(*new bsMeshRenderer(mMeshCache.getMesh("plane_1m.bsm"),
		mTextureCache.getDefaultTexture()));

	XMFLOAT4A halfExtents4A;
	XMStoreFloat4A(&halfExtents4A, halfExtents);

	hkVector4 halfExt;
	halfExt.load<4>(&halfExtents4A.x);

	hkpRigidBodyCinfo rbCinfo;
	//Create plane shape point upwards at origin.
	rbCinfo.m_shape = new hkpPlaneShape(hkVector4::getConstant<HK_QUADREAL_0100>(), hkVector4::getZero(), halfExt);
	rbCinfo.m_motionType = hkpMotion::MOTION_FIXED;

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	XMVECTOR scale = XMVectorScale(halfExtents, 2.0f);
	//Set Y scale to 1 regardless of input. Doesn't make sense to scale the Y axis of a
	//plane as it is infinitely thin.
	scale = XMVectorSetY(scale, 1.0f);
	entity->getTransform().setLocalScale(scale);

	return entity;
}

bsEntity* bsPrimitiveCreator::createCapsule(float height, float radius) const
{
	BS_ASSERT2(height > (radius * 2.0f), "The height of a capsule must always be larger"
		" than its radius");
	BS_ASSERT2(radius > 0.0f, "The radius of a capsule must be positive");

	//Capsule mesh dimensions: vertex0: (0, 0.5, 0), vertex1: (0, 1.5, 0), radius: 0.5

	bsEntity* entity = new bsEntity();
	entity->attachMeshRenderer(*new bsMeshRenderer(mMeshCache.getMesh("capsule.bsm"),
		mTextureCache.getDefaultTexture()));

	const hkVector4 vertexBottom(0.0f, radius, 0.0f);
	const hkVector4 vertexTop(0.0f, height - radius, 0.0f);
	//Capsules have no extra convex radius, no need to bake extra radius.

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpCapsuleShape(vertexBottom, vertexTop, radius);
	rbCinfo.m_motionType = hkpMotion::MOTION_DYNAMIC;

	//Compute capsule's mass properties.
	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(rbCinfo.m_shape,
		rbCinfo.m_mass, massProperties);
	massProperties.scaleToDensity(1.0f);
	rbCinfo.setMassProperties(massProperties);
	

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	//Mesh' radius is 0.5, so if radius parameter is 0.5, the scale needs to be 1,
	//so we multiply the radius by 2 to get the correct scaling factor.
	const float radius2 = radius * 2.0f;
	const float halfHeight = height * 0.5f;
	
	entity->getTransform().setScale(XMVectorSet(radius2, halfHeight, radius2, 0.0f));
	//entity->getTransform().scaleUniform(0.9);

	return entity;
}

bsEntity* bsPrimitiveCreator::createCylinder(float height, float radius) const
{
	//hkpCylinderShape::setNumberOfVirtualSideSegments(128);


	BS_ASSERT2(height > 0.0f, "The height of a cylinder must be positive");
	BS_ASSERT2(radius > 0.0f, "The radius of a cylinder must be positive");


	//Cylinder mesh dimensions: vertex0: (0, 0, 0), vertex1: (0, 2, 0), radius: 0.5

	bsEntity* entity = new bsEntity();
	entity->attachMeshRenderer(*new bsMeshRenderer(mMeshCache.getMesh("cylinder.bsm"),
		mTextureCache.getDefaultTexture()));

	//Bake convex radius into the cylinder's shape.
	const hkVector4 vertexBottom(0.0f, hkConvexShapeDefaultRadius, 0.0f);
	const hkVector4 vertexTop(0.0f, height - hkConvexShapeDefaultRadius, 0.0f);

	hkpRigidBodyCinfo rbCinfo;
	rbCinfo.m_shape = new hkpCylinderShape(vertexBottom, vertexTop,
		radius - hkConvexShapeDefaultRadius);//Baking convex radius into shape's radius.
	rbCinfo.m_motionType = hkpMotion::MOTION_DYNAMIC;

	//Compute capsule's mass properties.
	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(rbCinfo.m_shape,
		rbCinfo.m_mass, massProperties);
	massProperties.scaleToDensity(1.0f);
	rbCinfo.setMassProperties(massProperties);

	entity->attachRigidBody(*new hkpRigidBody(rbCinfo));

	//Mesh' radius is 0.5, so if radius parameter is 0.5, the scale needs to be 1,
	//so we multiply the radius by 2 to get the correct scaling factor.
	const float radius2 = radius * 2.0f;
	//Since the cylinder mesh' height is 2, height scale of 1 is only correct when height
	//is 2, so halve it to get the correct scaling factor.
	const float halfHeight = height * 0.5f;

	entity->getTransform().setScale(XMVectorSet(radius2, halfHeight, radius2, 0.0f));
	//entity->getTransform().scaleUniform(0.9);

	return entity;
}
