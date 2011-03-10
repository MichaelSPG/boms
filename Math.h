#ifndef MATH_H
#define MATH_H

#define XM_NO_OPERATOR_OVERLOADS//http://msdn.microsoft.com/en-us/library/ee418732%28v=VS.85%29.aspx#AvoidOverLoad
#include <xnamath.h>

namespace Math
{
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
	inline XMFLOAT4X4 XMFloat4x4Transpose(const XMFLOAT4X4& M)
	{
		const XMMATRIX& tempMatrix = XMMatrixTranspose(XMLoadFloat4x4(&M));

		XMFLOAT4X4 temp4x4;
		XMStoreFloat4x4(&temp4x4, tempMatrix);

		return temp4x4;
	}
}



#endif // MATH_H