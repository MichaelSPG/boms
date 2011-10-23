#include "StdAfx.h"

#include "bsFrustum.h"


bsFrustum bsComputeFrustumFromProjection(const XMMATRIX& projection)
{
	//Mostly taken from XNA::ComputeFrustumFromProjection() and
	//XNA::ComputePlanesFromFrustum().


	float rightSlope;	  // Positive X slope (X/Z).
	float leftSlope;	  // Negative X slope.
	float topSlope;		  // Positive Y slope (Y/Z).
	float bottomSlope;	  // Negative Y slope.
	float nearClip;		  // Z of the near plane.
	float farClip;		  // Z of the far plane.


	// Corners of the projection frustum in homogenous space.
	static const XMVECTOR homogenousPoints[6] =
	{
		{ 1.0f,  0.0f, 1.0f, 1.0f},   // right (at far plane)
		{-1.0f,  0.0f, 1.0f, 1.0f},   // left
		{ 0.0f,  1.0f, 1.0f, 1.0f},   // top
		{ 0.0f, -1.0f, 1.0f, 1.0f},   // bottom

		{0.0f, 0.0f, 0.0f, 1.0f},     // near
		{0.0f, 0.0f, 1.0f, 1.0f}      // far
	};

	XMVECTOR determinant;
	const XMMATRIX inverseProjection = XMMatrixInverse(&determinant, projection);

	// Compute the frustum corners in world space.
	XMVECTOR points[6];

	for (unsigned int i = 0; i < 6; ++i)
	{
		// Transform point.
		points[i] = XMVector4Transform(homogenousPoints[i], inverseProjection);
	}

	// Compute the slopes.
	points[0] = XMVectorMultiply(points[0], XMVectorReciprocal(XMVectorSplatZ(points[0])));
	points[1] = XMVectorMultiply(points[1], XMVectorReciprocal(XMVectorSplatZ(points[1])));
	points[2] = XMVectorMultiply(points[2], XMVectorReciprocal(XMVectorSplatZ(points[2])));
	points[3] = XMVectorMultiply(points[3], XMVectorReciprocal(XMVectorSplatZ(points[3])));

	XMVectorGetXPtr(&rightSlope, points[0]);
	XMVectorGetXPtr(&leftSlope, points[1]);
	XMVectorGetYPtr(&topSlope, points[2]);
	XMVectorGetYPtr(&bottomSlope, points[3]);

	// Compute near and far.
	points[4] = XMVectorMultiply(points[4], XMVectorReciprocal(XMVectorSplatW(points[4])));
	points[5] = XMVectorMultiply(points[5], XMVectorReciprocal(XMVectorSplatW(points[5])));

	XMVectorGetZPtr(&nearClip, points[4]);
	XMVectorGetZPtr(&farClip, points[5]);


	// Build the frustum planes.
	bsFrustum frustum;
	frustum.planes[0] = XMVectorSet( 0.0f,  0.0f, -1.0f, nearClip);
	frustum.planes[1] = XMVectorSet( 0.0f,  0.0f,  1.0f, -farClip);
	frustum.planes[2] = XMVectorSet( 1.0f,  0.0f, -rightSlope, 0.0f);
	frustum.planes[3] = XMVectorSet(-1.0f,  0.0f,  leftSlope, 0.0f);
	frustum.planes[4] = XMVectorSet( 0.0f,  1.0f, -topSlope, 0.0f);
	frustum.planes[5] = XMVectorSet( 0.0f, -1.0f,  bottomSlope, 0.0f);

	frustum.planes[0] = XMPlaneNormalize(frustum.planes[0]);
	frustum.planes[1] = XMPlaneNormalize(frustum.planes[1]);
	frustum.planes[2] = XMPlaneNormalize(frustum.planes[2]);
	frustum.planes[3] = XMPlaneNormalize(frustum.planes[3]);
	frustum.planes[4] = XMPlaneNormalize(frustum.planes[4]);
	frustum.planes[5] = XMPlaneNormalize(frustum.planes[5]);

	frustum.rightSlope = rightSlope;
	frustum.leftSlope = leftSlope;
	frustum.topSlope = topSlope;
	frustum.bottomSlope = bottomSlope;
	frustum.nearClip = nearClip;
	frustum.farClip = farClip;

	return frustum;
}

inline XMVECTOR transformPlane(const XMVECTOR& plane, const XMVECTOR& rotation,
	const XMVECTOR& translation)
{
	const XMVECTOR normal = XMVector3Rotate(plane, rotation);
	const XMVECTOR d = XMVectorSubtract(XMVectorSplatW(plane), XMVector3Dot(normal, translation));
	return XMVectorInsert(normal, d, 0, 0, 0, 0, 1);
}

bsFrustum bsTransformFrustum(const bsFrustum& frustum, const XMVECTOR& rotation,
	const XMVECTOR& translation)
{
	bsFrustum transformedFrustum;
	for (unsigned int i = 0; i < 6; ++i)
	{
		transformedFrustum.planes[i] =
			transformPlane(frustum.planes[i], rotation, translation);
	}
	for (unsigned int i = 0; i < 6; ++i)
	{
		transformedFrustum.planes[i] = XMPlaneNormalize(transformedFrustum.planes[i]);
	}

	transformedFrustum.rightSlope = frustum.rightSlope;
	transformedFrustum.leftSlope = frustum.leftSlope;
	transformedFrustum.topSlope = frustum.topSlope;
	transformedFrustum.bottomSlope = frustum.bottomSlope;
	transformedFrustum.nearClip = frustum.nearClip;
	transformedFrustum.farClip = frustum.farClip;

	/*
	const XMMATRIX m = XMMatrixMultiply(XMMatrixTranslationFromVector(translation),
		XMMatrixRotationQuaternion(rotation));

	XMPlaneTransformStream(reinterpret_cast<XMFLOAT4*>(transformedFrustum.planes),
		sizeof(XMVECTOR), reinterpret_cast<const XMFLOAT4*>(frustum.planes),
		sizeof(XMVECTOR), 6, m);
	*/
	return transformedFrustum;
}
