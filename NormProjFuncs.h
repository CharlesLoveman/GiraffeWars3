#ifndef _NORMPROJFUNCS_H_
#define _NORMPROJFUNCS_H_
#include "Giraffe.h"

struct NormProjFuncs {
	static void SpitOnHit(Projectile& self, Giraffe& parent, Giraffe* collided)
	{
		//Do nothing
	}
	static void SpitUpdate(Projectile& self, Giraffe& parent)
	{
		self.Velocity.y += parent.Gravity;
	}
	static void SpitDraw(Projectile& self, Giraffe& parent, HDC hdc, Vec2 Scale)
	{
		SelectObject(hdc, self.Pen);
		SelectObject(hdc, self.Brush);
		Ellipse(hdc, Scale.x * (self.Position.x - self.Radius), Scale.y * (self.Position.y - self.Radius), Scale.x * (self.Position.x + self.Radius), Scale.y * (self.Position.y + self.Radius));
	}
	static void NeckGrabOnHit(Projectile& self, Giraffe& parent, Giraffe* collided)
	{
		parent.Velocity += Vec2(0.2f, 0.3f) * (self.Position - parent.Position);
	}
	static void NeckGrabUpdate(Projectile& self, Giraffe& parent)
	{
		//No Special Updates;
	}
	static void NeckGrabDraw(Projectile& self, Giraffe& parent, HDC hdc, Vec2 Scale)
	{
		SelectObject(hdc, self.Pen);
		POINT points[5];

		for (int i = 0; i < 5; ++i) {
			points[i] = (Scale * (self.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[29 + i])).ToPoint();
		}

		Polyline(hdc, points, 5);
	}
};

#endif
