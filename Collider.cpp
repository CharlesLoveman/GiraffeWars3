#include "Collider.h"


HitCollider::HitCollider()
{
	Position = { 0, 0 };
	Radius = 1.0f;
	Force = { 1.0f, 1.0f };
	Damage = 1.0f;
	Knockback = 1.0f;
	Scale = 1.0f;
	Fixed = false;
}

HitCollider::HitCollider(Vec2 Pos, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix)
{
	Position = Pos;
	Radius = Rad;
	Force = Frc;
	Damage = Dmg;
	Knockback = Knk;
	Scale = Scl;
	Fixed = Fix;
}

HurtCollider::HurtCollider()
{
	Position = { 0,0 };
	Radius = 1.0f;
	DamageMultiplier = 1.0f;
}

HurtCollider::HurtCollider(Vec2 Pos, float Rad, float DmgMult)
{
	Position = Pos;
	Radius = Rad;
	DamageMultiplier = DmgMult;
}

bool Intersect(Vec2 pos1, Collider col1, Vec2 fac1, Vec2 pos2, Collider col2, Vec2 fac2)
{
	return ((pos1 + col1.Position * fac1) - (pos2 + col2.Position * fac2)).Length() < (col1.Radius + col2.Radius);
}
