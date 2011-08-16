#pragma once

#include "bsConfig.h"

#include <Common/Base/hkBase.h>

#include <Windows.h>
#include <xnamath.h>


/*	Functions to convert between XNA math and Havok math classes.
	
*/
namespace bsMath
{

//Vectors

inline XMFLOAT3 toXM3(const hkVector4& vec)
{
	XMFLOAT3 temp;
	vec.storeNotAligned<3>(&temp.x);
	return temp;
}

inline XMFLOAT4 toXM4(const hkVector4& vec)
{
	XMFLOAT4 temp;
	vec.storeNotAligned<4>(&temp.x);
	return temp;
}

inline hkVector4 toHK(const XMFLOAT3& vec)
{
	hkVector4 temp;
	temp.loadNotAligned<3>(&vec.x);
	return temp;
}

inline hkVector4 toHK(const XMFLOAT4& vec)
{
	hkVector4 temp;
	temp.loadNotAligned<4>(&vec.x);
	return temp;
}

//Matrices

inline hkTransform toHK(const XMFLOAT4X4& transform)
{
	hkRotation rot;
	rot.setCols(hkVector4(transform._11, transform._21, transform._31, transform._41),
		hkVector4(transform._12, transform._22, transform._32, transform._42),
		hkVector4(transform._13, transform._23, transform._33, transform._43));

	return hkTransform(rot, hkVector4(transform._14, transform._24, transform._34));
}

inline XMFLOAT4X4 toXM(const hkTransform& transform)
{
	XMFLOAT4X4 xmTransform;
	transform.get4x4ColumnMajor(&xmTransform.m[0][0]);
	return xmTransform;
}

} // namespace bsMath
