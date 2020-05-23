#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "Collider.h"
class Giraffe;
struct Projectile;

typedef void (*ProjectileFunc)(Projectile&, Giraffe&, Giraffe*);

struct Projectile : public HitCollider {
public:
	Projectile();
	Projectile(Vec2 Pos, Vec2 Vel, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID, ProjectileFunc _OnHit);
	Vec2 Velocity;
	int ID;
	ProjectileFunc OnHit;
};

#endif // !_PROJECTILE_H_
