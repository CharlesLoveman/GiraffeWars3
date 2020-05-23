#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "Collider.h"
#include <Windows.h>
class Giraffe;
class Projectile;

typedef void (*ProjectileOnHit)(Projectile&, Giraffe&, Giraffe*);
typedef void (*ProjectileUpdate)(Projectile&, Giraffe&);
typedef void (*ProjectileDraw)(Projectile&, Giraffe&, HDC, Vec2);

class Projectile : public HitCollider {
public:
	Projectile();
	Projectile(Vec2 Pos, Vec2 Vel, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID, ProjectileOnHit _OnHit, ProjectileUpdate _Update, ProjectileDraw _Draw, HPEN _Pen, HBRUSH _Brush);
	Vec2 Velocity;
	int ID;
	ProjectileOnHit OnHit;
	ProjectileUpdate Update;
	ProjectileDraw Draw;
	HPEN Pen;
	HBRUSH Brush;
};

#endif // !_PROJECTILE_H_
