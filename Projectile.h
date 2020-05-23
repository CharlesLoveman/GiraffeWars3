#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "Collider.h"

struct Projectile : public HitCollider {
public:
	Projectile();
	Projectile(Vec2 Pos, Vec2 Vel, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix, int _ID);
	Vec2 Velocity;
	int ID;
};

#endif // !_PROJECTILE_H_
