#ifndef CONVERT_H
#define CONVERT_H

#include <Common/Base/hkBase.h>

#include "bsMath.h"

namespace bsMath
{

inline XMFLOAT3 toXM(const hkVector4& vec)
{
	return XMFLOAT3(vec.getSimdAt(0), vec.getSimdAt(1), vec.getSimdAt(2));
}

inline hkVector4 toHK(const XMFLOAT3& vec)
{
	return hkVector4(vec.x, vec.y, vec.z, 0.0f);
}

} // bsMath

#endif // CONVERT_H