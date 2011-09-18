#pragma once


#include <Common/Base/hkBase.h>

#include <Windows.h>
#include <xnamath.h>


/*	Functions to convert between XNA math and Havok math classes.
	
*/
namespace bsMath
{

//Vectors

inline XMVECTOR toXM(const hkVector4& vec)
{
	XMVECTOR temp;
	vec.store<4>(temp.m128_f32);
	return temp;
}

inline hkVector4 toHK(const XMVECTOR& vec)
{
	hkVector4 temp;
	temp.load<4>(vec.m128_f32);
	return temp;
}

//Matrices

inline hkTransform toHK(const XMMATRIX& m)
{
	XMFLOAT4X4A transform;
	XMStoreFloat4x4A(&transform, m);

	hkQuaternion q;
	q.m_vec = toHK(XMQuaternionRotationMatrix(m));

	/*hkRotation rot;
	
	rot.setCols(hkVector4(transform._11, transform._21, transform._31, transform._41),
		hkVector4(transform._12, transform._22, transform._32, transform._42),
		hkVector4(transform._13, transform._23, transform._33, transform._43));
	*/
	return hkTransform(q, hkVector4(transform._14, transform._24, transform._34));
}

inline XMMATRIX toXM(const hkTransform& transform)
{
	XMFLOAT4X4A xmTransform;
	transform.get4x4ColumnMajor(&xmTransform.m[0][0]);
	
	return XMLoadFloat4x4A(&xmTransform);
}


//Quaternion

inline XMVECTOR toXM(const hkQuaternion& rotation)
{
	XMFLOAT4A xmQuat;
	rotation.m_vec.store<4>(&xmQuat.x);
	return XMLoadFloat4A(&xmQuat);
}


} // namespace bsMath
