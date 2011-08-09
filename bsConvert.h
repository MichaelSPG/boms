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
	return XMFLOAT3(vec.getComponent<0>().getReal(), vec.getComponent<1>().getReal(),
		vec.getComponent<2>().getReal());
}

inline XMFLOAT4 toXM4(const hkVector4& vec)
{
	return XMFLOAT4(vec.getComponent<0>().getReal(), vec.getComponent<1>().getReal(),
		vec.getComponent<2>().getReal(), vec.getComponent<3>().getReal());
}

inline hkVector4 toHK(const XMFLOAT3& vec)
{
	return hkVector4(vec.x, vec.y, vec.z, 0.0f);
}

inline hkVector4 toHK(const XMFLOAT4& vec)
{
	return hkVector4(vec.x, vec.y, vec.z, vec.w);
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
