#pragma once

class NETLIB_API Vector3 final
{
public:
	Vector3()
	{
		x = 0.0f; y = 0.0f; z = 0.0f;
	}
	Vector3(float _x, float _y, float _z)
	{
		x = _x; y = _y; z = _z;
	}
	~Vector3() = default;

	Vector3 operator+(const Vector3& o) { return Vector3(x + o.x, y + o.y, z + o.z); }
	Vector3 operator+(const Vector3& o) const { return Vector3(x + o.x, y + o.y, z + o.z); }
	Vector3& operator+=(const Vector3& o)
	{
		x += o.x;
		y += o.y;
		z += o.z;
		return *this;
	}
	Vector3 operator-(const Vector3& o) { return Vector3(x - o.x, y - o.y, z - o.z); }
	Vector3 operator-(const Vector3& o) const { return Vector3(x - o.x, y - o.y, z - o.z); }
	Vector3& operator-=(const Vector3& o)
	{
		x -= o.x;
		y -= o.y;
		z -= o.z;
		return *this;
	}
	Vector3 operator*(const float s) { return Vector3(x * s, y * s, z * s); }
	Vector3 operator*(const float s) const { return Vector3(x * s, y * s, z * s); }
	Vector3& operator*=(const float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
	Vector3 operator/(const float s) { return Vector3(x / s, y / s, z / s); }
	Vector3 operator/(const float s) const { return Vector3(x / s, y / s, z / s); }
	Vector3& operator/=(const float s)
	{
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}



	static float Dot(const Vector3& a, const Vector3& b)
	{
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	}
	static Vector3 Cross(const Vector3& a, const Vector3& b)
	{
		float _x = a.y * b.z - b.y * a.z;
		float _y = a.z * b.x - b.z * a.x;
		float _z = a.x * b.y - b.x * a.y;
		return Vector3(_x, _y, _z);
	}
	static float Distance(const Vector3& a, const Vector3& b)
	{
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
	}
	static float Angle(const Vector3& a, const Vector3& b)
	{
		return atan2(fabs(a.x - b.x), fabs(a.z - b.z)) * RAD2DEG;
	}
	static Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta)
	{
		Vector3 a = target - current;
		float magnitude = a.magnitude();
		if (magnitude <= maxDistanceDelta || 0.0f == magnitude)
			return target;
		return current + a / magnitude * maxDistanceDelta;
	}

	// 부채꼴 체크
	static bool CheckSector(const Vector3& current, const Vector3& target, const Vector3& forward, unsigned short angle, float maxDistance)
	{
		/*
		Vector3 direction = (target - current).normalized();
		if (Dot(forward, direction) >= cos((viewAngle >> 1) * DEG2RAD))
		{
			float distance = Distance(current, target);
			if(maxDistance >= distance)
				return true;
		}
		*/

		if (maxDistance >= Distance(current, target))
		{
			Vector3 direction = (target - current).normalized();
			if (Dot(forward, direction) >= cos((angle >> 1) * DEG2RAD))
			{
				return true;
			}
		}

		return false;
	}





	float magnitude() { return sqrt(sqrMagnitude()); }
	float sqrMagnitude() { return (x * x) + (y * y) + (z * z); }
	Vector3 normalized()
	{
		float length = magnitude();
		if (0.0f == length)
			return *this;
		return (*this) / length;
	}

	void Zero()
	{
		x = 0.0f; y = 0.0f; z = 0.0f;
	}

	float x; float y; float z;
};