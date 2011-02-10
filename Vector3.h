#ifndef VECTOR_3_H
#define VECTOR_3_H

#include <assert.h>
#include <math.h>


class Vector3
{
public:
	inline Vector3() {}

	/**	Copy constructor.
	*/
	inline Vector3(const Vector3 &vec) : 
		x(vec.x),
		y(vec.y),
		z(vec.z)
	{}

	inline Vector3(float X, float Y, float Z) : 
		x(X),
		y(Y),
		z(Z)
	{}

	inline Vector3& operator=(const Vector3 &vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;

		return *this;
	}

	inline bool operator==(const Vector3 &vec)
	{
		return x == vec.x && y == vec.y && z == vec.z;
	}

	inline bool operator!=(const Vector3 &vec)
	{
		return x != vec.x && y != vec.y && z != vec.z;
	}

	/**	Set every element to zero.
	*/
	inline void zero()
	{
		x = y = z = 0.0f;
	}

	inline Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	inline Vector3 operator+(const Vector3 &vec) const
	{
		return Vector3(x + vec.x, y + vec.y, z + vec.z);
	}

	inline Vector3 operator-(const Vector3 &vec) const
	{
		return Vector3(x - vec.x, y - vec.y, z - vec.z);
	}

	inline Vector3 operator*(float f) const
	{
		return Vector3(x * f, y * f, z * f);
	}

	inline Vector3 operator/(float f) const
	{
		assert (f != 0.0f && "Cannot divide vector by zero");

		float oneOverF = 1.0f / f;

		return Vector3(x * oneOverF, y * oneOverF, z * oneOverF);
	}

	inline Vector3& operator+=(const Vector3 &vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;

		return *this;
	}

	inline Vector3& operator-=(const Vector3 &vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;

		return *this;
	}

	inline Vector3& operator *=(const float f)
	{
		x *= f;
		y *= f;
		z *= f;

		return *this;
	} 

	inline 	Vector3& operator/=(const float f)
	{
		float oneOverF = 1.0f / f;

		x *= oneOverF;
		y *= oneOverF;
		z *= oneOverF;

		return *this;
	}

	/**	Requires square root.
	*/
	inline void normalize()
	{
		float squared = (x * x) + (y * y) + (z * z);

		if (squared > 0.0f)
		{
			float oneOverSquared(1.0f / sqrtf(squared));

			x *= oneOverSquared;
			y *= oneOverSquared;
			z *= oneOverSquared;
		}
	}

	inline float operator *(const Vector3 &vec) const
	{
		return (x * vec.x) + (y * vec.y) + (z + vec.z);
	}

	/**	Requires square root.
	*/
	inline float magnitude()
	{
		return sqrtf((x * x) + (y * y) + (z * z));
	}

	/**	Returns a vector that lies perpendicular to this and the parameter.
	*/
	inline Vector3 crossProduct(const Vector3 &vec)
	{
		return Vector3(
			(y * vec.z) + (z * vec.y),
			(z * vec.x) + (x * vec.z),
			(x * vec.y) + (y * vec.x));
	}

	

	/**	Requires square root.
	*/
	inline float distance(const Vector3 &vec)
	{
		float dx = x - vec.x;
		float dy = y - vec.y;
		float dz = z - vec.z;

		return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
	}


	float x, y, z;
};

inline Vector3 operator*(float f, const Vector3 &vec)
{
	return Vector3(f * vec.x, f * vec.y, f * vec.z);
}

#endif