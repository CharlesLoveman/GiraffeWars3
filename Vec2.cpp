#include "Vec2.h"
#include <math.h>

Vec2::Vec2()
{
	x = 0;
	y = 0;
}

Vec2::Vec2(float _x, float _y)
{
	x = _x;
	y = _y;
}

Vec2& Vec2::operator+=(const Vec2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vec2& Vec2::operator-=(const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vec2& Vec2::operator*=(const Vec2& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	return *this;
}

Vec2& Vec2::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	return *this;
}

float Vec2::Length()
{
	return sqrtf(x * x + y * y);
}

Vec2 Vec2::Normalise()
{
	return (1 / Length()) * *this;
}

Vec2 Vec2::GetPerpendicular()
{
	return Vec2(-y, x);
}

float Vec2::Dot(const Vec2 rhs)
{
	return x * rhs.x + y * rhs.y;
}

POINT Vec2::ToPoint()
{
	return { (int)x, (int)y };
}
