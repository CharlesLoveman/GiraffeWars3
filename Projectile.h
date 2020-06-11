#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "Collider.h"
#include <Windows.h>
class Giraffe;
class Projectile;

typedef void (*ProjectileOnHit)(Projectile&, Giraffe&, Giraffe*, int);
typedef bool (*ProjectileUpdate)(Projectile&, Giraffe&, int);
typedef void (*ProjectileDraw)(Projectile&, Giraffe&, HDC, Vector2, int);

class Projectile : public HitCollider {
public:
	Projectile();
	Projectile(Vector2 Pos, Vector2 Vel, float Rad, Vector2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID, int _LifeSpan, ProjectileOnHit _OnHit, ProjectileUpdate _Update, ProjectileDraw _Draw, HPEN _Pen, HBRUSH _Brush);
	Vector2 Velocity;
	int ID;
	int LifeSpan;
	ProjectileOnHit OnHit;
	ProjectileUpdate Update;
	ProjectileDraw Draw;
	HPEN Pen;
	HBRUSH Brush;
	int Checksum();
};

#endif // !_PROJECTILE_H_
