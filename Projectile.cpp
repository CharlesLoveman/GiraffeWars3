#include "Projectile.h"
#include "giraffewar.h"

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
	LifeSpan = 0;
	OnHit = nullptr;
	Update = nullptr;
	Draw = nullptr;
	Pen = nullptr;
	Brush = nullptr;
}

Projectile::Projectile(Vector2 Pos, Vector2 Vel, float Rad, Vector2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID, int _LifeSpan, ProjectileOnHit _OnHit, ProjectileUpdate _Update, ProjectileDraw _Draw, HPEN _Pen, HBRUSH _Brush)
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
	LifeSpan = _LifeSpan;
	OnHit = _OnHit;
	Update = _Update;
	Draw = _Draw;
	Pen = _Pen;
	Brush = _Brush;
}

int Projectile::Checksum()
{
	return fletcher32_checksum((short*)&Position, (size_t)((size_t)&LifeSpan - (size_t)&Position) / 2);
}
