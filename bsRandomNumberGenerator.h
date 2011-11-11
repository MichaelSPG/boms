#pragma once

#include <xnamath.h>
#include "bsAssert.h"


/*	Very basic random number generator. Can generate random vectors and rotations.

	Based on fast_rand from
	http://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/
*/
class bsRandomNumberGenerator
{
public:
	bsRandomNumberGenerator(unsigned int seed)
		: mSeed(seed)
	{}

	//Unsigned int in range [0, UINT_MAX).
	unsigned int uint()
	{
		mSeed = 214013 * mSeed + 2531011;
		return mSeed;
	}

	//Float in range [0, 1].
	float float01()
	{
		const unsigned int randomUint32 = uint();
		return (float)randomUint32 / (float)UINT_MAX;
	}

	//Float in range [-1, 1].
	float float11()
	{
		return float01() * 2.0f - 1.0f;
	}

	//Float in range [min, max].
	float range(float min, float max)
	{
		BS_ASSERT2(min < max, "Min must be less than max");

		const float f01 = float01();
		return min + f01 * (max - min);
	}

	//Returns int in range [min, max)
	int range(int min, int max)
	{
		BS_ASSERT2(min < max, "Min must be less than max");

		const float f01 = float01();
		return (int)(min + f01 * (max - min));
	}

	//Vector with all 4 components in range [0, 1].
	XMVECTOR vector01()
	{
		return XMVectorSet(float01(), float01(), float01(), float01());
	}

	//Vector with all 4 components in range [-1, 1].
	XMVECTOR vector11()
	{
		return XMVectorSet(float11(), float11(), float11(), float11());
	}

	//Vector with components in range [min, max].
	//Returned vector will have each of its components in ranges described by parameters.
	//x = randomRange(min.x, max.x), etc.
	XMVECTOR vectorRange(const XMVECTOR& min, const XMVECTOR& max)
	{
		BS_ASSERT2(XMVector4Less(min, max), "Min must be less than max");

		const XMVECTOR delta = XMVectorSubtract(max, min);
		const XMVECTOR v01 = vector01();
		return XMVectorAdd(min, XMVectorMultiply(delta, v01));
	}

	//Vector wtih all 4 components in range [min, max].
	XMVECTOR vectorRange(float min, float max)
	{
		BS_ASSERT2(min < max, "Min must be less than max");

		return XMVectorSet(range(min, max), range(min, max),
			range(min, max), range(min, max));
	}

	//Quaternion with normalized random rotation.
	XMVECTOR rotation()
	{
		const XMVECTOR rot = XMVectorSet(float11(), float11(), float11(),
			float11());
		return XMQuaternionNormalize(rot);
	}

private:
	unsigned int mSeed;
};
