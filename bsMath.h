#pragma once

#include <math.h>
#include <stdlib.h>//rand()

#include <Windows.h>//xnamath.h needs defines from this header, such as FLOAT
#include <xnamath.h>

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

/*	Compare two floats to see if they're within an epsilon of eachother, returing true if so.
*/
inline bool approximatelyEqual(float number1, float number2, float epsilon = 0.00001f)
{
	return (fabsf(number1 - number2) < epsilon);
}

/*	Compare two vectors to see if each x, y and z element is within an epsilon of the
	other vector's x, y and z elements.
	Returns true only if all three elements are within the epsilon of eachother.
*/
inline bool approximatelyEqual(const XMFLOAT3& vec1, const XMFLOAT3& vec2,
	float epsilon = 0.00001f)
{
	return approximatelyEqual(vec1.x, vec2.x, epsilon)
		&& approximatelyEqual(vec1.y, vec2.y, epsilon)
		&& approximatelyEqual(vec1.z, vec2.z, epsilon);
}

/*	Linearly interpolates between min and max given a ratio in range [0, 1].
	A ratio of 0 will return the min input, while a ratio of 1 will return the max input.
*/
inline float lerp(float min, float max, float ratio)
{
	float delta = max - min;
	return min + (ratio * delta);
}

/*	Returns value clamped to be less than or equal to max and greater than or equal to min.	
*/
inline float clamp(float min, float max, float value)
{
	return XMMin(XMMax(min, value), max);
}
} // namespace bsMath
