#ifndef MATH_H
#define MATH_H

#define XM_NO_OPERATOR_OVERLOADS//http://msdn.microsoft.com/en-us/library/ee418732%28v=VS.85%29.aspx#AvoidOverLoad
#include <xnamath.h>

namespace Math
{
	inline XMFLOAT3 getTranslationFromMatrix(const XMMATRIX& matrix)
	{
		return XMFLOAT3(matrix._41, matrix._42, matrix._43);
	}

	inline XMFLOAT3 getTranslationFromFloat4x4(const XMFLOAT4X4& matrix)
	{
		return XMFLOAT3(matrix._41, matrix._42, matrix._43);
	}

	inline void setTranslationInMatrix(XMMATRIX& matrix, const XMVECTOR& translation)
	{
		matrix._41 = -XMVectorGetX(translation);
		matrix._42 = -XMVectorGetY(translation);
		matrix._43 = -XMVectorGetZ(translation);
	}

	inline void setTranslationInFloat4x4(XMFLOAT4X4& matrix, const XMVECTOR& translation)
	{
		matrix._41 = -XMVectorGetX(translation);
		matrix._42 = -XMVectorGetY(translation);
		matrix._43 = -XMVectorGetZ(translation);
	}
	
	inline void setTranslationInMatrix(XMMATRIX& matrix, const XMFLOAT3& translation)
	{
		matrix._41 = -translation.x;
		matrix._42 = -translation.y;
		matrix._43 = -translation.z;
	}

	inline void setTranslationInFloat4x4(XMFLOAT4X4& matrix, const XMFLOAT3& translation)
	{
		matrix._41 = -translation.x;
		matrix._42 = -translation.y;
		matrix._43 = -translation.z;
	}


}



#endif // MATH_H