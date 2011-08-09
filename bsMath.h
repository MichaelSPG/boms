#pragma once

#include <math.h>
#include <stdlib.h>//rand()

#include <Windows.h>//xnamath.h needs defines from this header, such as FLOAT
#define XM_NO_OPERATOR_OVERLOADS//http://msdn.microsoft.com/en-us/library/ee418732%28v=VS.85%29.aspx#AvoidOverLoad
#include <xnamath.h>

#include <Common/Base/hkBase.h>
#include <Common/Base/Math/hkMath.h>

#include "bsConvert.h"


/*	Contains some mathematical functions for random number generations and some
	wrapper functions for converting non-SIMD XNA structs to SIMD-structs XNA structs
	and performing operations with SIMD.
*/
namespace bsMath
{
//Uses C rand(). Remember to call srand().
inline float randomRange(float min, float max)
{
	//r in range [0, 1]
	float r = (float)rand() / RAND_MAX;
	return min + r * (max - min);
}

//Uses C rand(). Remember to call srand().
inline int randomRange(int min, int max)
{
	float r = (float)rand() / RAND_MAX;
	r = min + r * (max - min);
	//Add positive or negative 0.5 depending on r's sign to round properly
	return (int)(r + (r < 0.0f ? -0.5f : 0.5f));
}


//Translation
//Get
inline XMFLOAT3 XMMatrixGetTranslation(const XMMATRIX& matrix)
{
	return XMFLOAT3(matrix._41, matrix._42, matrix._43);
}
	
//Set
inline void XMMatrixSetTranslation(XMMATRIX& matrix, const XMVECTOR& translation)
{
	matrix._41 = -XMVectorGetX(translation);
	matrix._42 = -XMVectorGetY(translation);
	matrix._43 = -XMVectorGetZ(translation);
}

inline void XMMatrixSetTranslation(XMMATRIX& matrix, const XMFLOAT3& translation)
{
	matrix._41 = -translation.x;
	matrix._42 = -translation.y;
	matrix._43 = -translation.z;
}
	
inline void XMFloat4x4SetTranslation(XMFLOAT4X4& matrix, const XMVECTOR& translation)
{
	matrix._41 = -XMVectorGetX(translation);
	matrix._42 = -XMVectorGetY(translation);
	matrix._43 = -XMVectorGetZ(translation);
}

inline void XMFloat4x4SetTranslation(XMFLOAT4X4& matrix, const XMFLOAT3& translation)
{
	matrix._41 = -translation.x;
	matrix._42 = -translation.y;
	matrix._43 = -translation.z;
}

//Transpose
//The parameter will be modified.
inline void XMFloat4x4Transpose(XMFLOAT4X4& inOut)
{
	XMStoreFloat4x4(&inOut, XMMatrixTranspose(XMLoadFloat4x4(&inOut)));
}

//The result will be put into the out parameter.
inline void XMFloat4x4Multiply(const XMFLOAT4X4& M1, const XMFLOAT4X4& M2, XMFLOAT4X4& out)
{
	XMStoreFloat4x4(&out, XMMatrixMultiply(XMLoadFloat4x4(&M1), XMLoadFloat4x4(&M2)));
}
	
//The result will be put into the out parameter.
inline void XMFloat4x4PerspectiveFovLH(float fovAngleY, float aspectRatio,
	float nearZ, float farZ, XMFLOAT4X4& out)
{
	XMStoreFloat4x4(&out, XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ));
}

inline XMFLOAT4X4 XMFloat4x4Identity()
{
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	return identity;
}
} // namespace bsMath
