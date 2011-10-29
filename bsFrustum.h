#pragma once

#include <xnamath.h>


__declspec(align(16)) struct bsFrustum
{
	/*	Near, far, right, left, top and bottom frustum planes.
	*/
	XMVECTOR planes[6];
};

/*	Creates a frustum with 6 planes from a projection matrix.
	The projection matrix should not contain rotation, translation or scaling.
*/
bsFrustum bsComputeFrustumFromProjection(const XMMATRIX& projection);

/*	Returns a transformed version of the input frustum.
*/
bsFrustum bsTransformFrustum(const bsFrustum& frustum, const XMVECTOR& rotation,
	const XMVECTOR& translation);
