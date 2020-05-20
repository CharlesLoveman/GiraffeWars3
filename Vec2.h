#ifndef _VEC2_H_
#define _VEC2_H_

#include <Windows.h>

struct Vec2 {
	float x;
	float y;

public:
	Vec2();
	Vec2(float _x, float _y);
	Vec2& operator+=(const Vec2& rhs);
	Vec2& operator-=(const Vec2& rhs);
	Vec2& operator*=(const Vec2& rhs);
	Vec2& operator*=(const float rhs);

	float Length();
	Vec2 Normalise();
	Vec2 GetPerpendicular();
	float Dot(const Vec2 rhs);
	POINT ToPoint();

	friend Vec2 operator +(Vec2 lhs, const Vec2& rhs)
	{
		lhs += rhs;
		return lhs;
	}

	friend Vec2 operator-(Vec2 lhs, const Vec2& rhs)
	{
		lhs -= rhs;
		return lhs;
	}

	friend Vec2 operator*(Vec2 lhs, const Vec2& rhs)
	{
		lhs *= rhs;
		return lhs;
	}

	friend Vec2 operator*(Vec2 lhs, const float rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend Vec2 operator*(const float lhs, Vec2 rhs) {
		rhs *= lhs;
		return rhs;
	}
};

#endif // !_VEC2_H_
