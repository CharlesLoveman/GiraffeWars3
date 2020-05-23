#include "Projectile.h"

Projectile::Projectile()
{
	Position = { 0, 0 };
	Velocity = { 0, 0 };
	Radius = 1.0f;
	Force = { 1.0f, 1.0f };
	Damage = 1.0f;
	Knockback = 1.0f;
	Scale = 1.0f;
	Fixed = false;
	ID = 0;
	OnHit = nullptr;
}

Projectile::Projectile(Vec2 Pos, Vec2 Vel, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID, ProjectileFunc _OnHit)
{
	Position = Pos;
	Velocity = Vel;
	Radius = Rad;
	Force = Frc;
	Damage = Dmg;
	Knockback = Knk;
	Scale = Scl;
	Fixed = Fix;
	ID = _ID;
	OnHit = _OnHit;
}
