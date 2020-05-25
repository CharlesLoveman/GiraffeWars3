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

HitCollider::HitCollider(Vector2 Pos, float Rad, Vector2 Frc, float Dmg, float Knk, float Scl, bool Fix)
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

HurtCollider::HurtCollider(Vector2 Pos, float Rad, float DmgMult)
{
	Position = Pos;
	Radius = Rad;
	DamageMultiplier = DmgMult;
}

bool Intersect(Vector2 pos1, Collider col1, Vector2 fac1, Vector2 pos2, Collider col2, Vector2 fac2)
{
	return Vector2::DistanceSquared(pos1 + col1.Position * fac1, pos2 + col2.Position * fac2) < col1.Radius * col1.Radius + col2.Radius * col2.Radius;
}
