#pragma once


#include <Common/Base/hkBase.h>
#include <Common/Base/Math/hkMath.h>

#include <Windows.h>
#include <xnamath.h>


/*	Functions to convert between XNA math and Havok math classes.
*/
namespace bsMath
{

//Vectors

inline XMVECTOR toXM(const hkVector4& vec)
{
	XMFLOAT4A temp;
	vec.store<4, HK_IO_SIMD_ALIGNED>(&temp.x);
	return XMLoadFloat4A(&temp);
}

inline hkVector4 toHK(const XMVECTOR& vec)
{
	XMFLOAT4A temp;
	XMStoreFloat4A(&temp, vec);

	hkVector4 hkVec;
	hkVec.load<4, HK_IO_SIMD_ALIGNED>(&temp.x);
	return hkVec;
}

//Matrices

inline hkTransform toHK(const XMMATRIX& m)
{
	XMFLOAT4X4A transform;
	XMStoreFloat4x4A(&transform, m);

	hkVector4 col1, col2, col3, col4;
	col1.load<4, HK_IO_SIMD_ALIGNED>(&transform.m[0][0]);
	col2.load<4, HK_IO_SIMD_ALIGNED>(&transform.m[1][0]);
	col3.load<4, HK_IO_SIMD_ALIGNED>(&transform.m[2][0]);
	col4.load<4, HK_IO_SIMD_ALIGNED>(&transform.m[3][0]);

	hkTransform t;
	t.setColumn<0>(col1);
	t.setColumn<1>(col2);
	t.setColumn<2>(col3);
	t.setColumn<3>(col4);

	return t;
}

inline XMMATRIX toXM(const hkTransform& transform)
{
	XMFLOAT4X4A xmTransform;
	transform.getColumn<0>().store<4, HK_IO_SIMD_ALIGNED>(xmTransform.m[0]);
	transform.getColumn<1>().store<4, HK_IO_SIMD_ALIGNED>(xmTransform.m[1]);
	transform.getColumn<2>().store<4, HK_IO_SIMD_ALIGNED>(xmTransform.m[2]);
	transform.getColumn<3>().store<4, HK_IO_SIMD_ALIGNED>(xmTransform.m[3]);

	return XMLoadFloat4x4A(&xmTransform);
}


//Quaternion

inline XMVECTOR toXM(const hkQuaternion& rotation)
{
	XMFLOAT4A xmQuat;
	rotation.m_vec.store<4, HK_IO_SIMD_ALIGNED>(&xmQuat.x);
	return XMLoadFloat4A(&xmQuat);
}

//3x3 rotation matrix to quaternion.
inline XMVECTOR toXM(const hkRotation& rotation)
{
	return toXM(hkQuaternion(rotation));
}

} // namespace bsMath
