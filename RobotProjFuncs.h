#ifndef _ROBOTPROJFUNCS_H_
#define _ROBOTPROJFUNCS_H_

#include "Giraffe.h"
#include <math.h>

struct RobotProjFuncs {

	static void GrabberOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		if (collided != nullptr) {
			if (collided->GrabHit(self, parent.Facing, frameNumber)) {
				parent.State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
				parent.State |= STATE_GRABBING;
				parent.TechDelay = frameNumber + 30;
				collided->Velocity = (parent.Position - collided->Position) * 0.1f;
			}
		}
	}

	static bool GrabberUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}

	static void GrabberDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {

		std::vector<Vector2>* skel = parent.Moves->GetSkelPoints(12, 11);
		Vector2 toGrabber = (self.Position - (parent.Position + parent.Facing * ((*skel)[27] + (*skel)[28]) * 0.5f));
		toGrabber.Normalize();
		Vector2 grabPerp = { -toGrabber.y, toGrabber.x };
		POINT points[8];
		points[0] = Giraffe::VecToPoint((parent.Position + parent.Facing * ((*skel)[27] + (*skel)[28]) * 0.5f), Scale);
		points[1] = Giraffe::VecToPoint(self.Position, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * 0.5f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + toGrabber * 0.4f + grabPerp * 0.5f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * 0.5f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * -0.5f, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + toGrabber * 0.4f + grabPerp * -0.5f, Scale);

		Polyline(hdc, points, 8);
	}

	static void MissileOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Add explosion projectile to parent
	}

	static bool MissileUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		Vector2 newVel;
		float theta = 0.1f * (2 * (((frameNumber + (int)trunc(self.Position.y * 10)) % (((int)(self.Position.x) ^ self.ID) + 1)) % 3) - 1);
		newVel.x = self.Velocity.Dot({ cosf(theta), -sinf(theta) });
		newVel.y = self.Velocity.Dot({ sinf(theta), cosf(theta) });
		self.Velocity = newVel;
		return frameNumber >= self.LifeSpan;
	}

	static void MissileDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Brush);
		SelectObject(hdc, self.Pen);
		Ellipse(hdc, Scale.x * (self.Position.x - 1.0f), Scale.y * (self.Position.y - 1.0f), Scale.x * (self.Position.x + 1.0f), Scale.y * (self.Position.y + 1.0f));
	}
};

#endif // !_ROBOTPROJFUNCS_H_
