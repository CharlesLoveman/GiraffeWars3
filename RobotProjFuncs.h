#ifndef _ROBOTPROJFUNCS_H_
#define _ROBOTPROJFUNCS_H_

#include "Giraffe.h"
#include <math.h>

struct RobotProjFuncs {

	static void StandardOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Do nothing?
	}

	static bool StandardUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}

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
		parent.Projectiles.Append(Projectile(self.Position, Vector2::Zero, 0, Vector2::Zero, 0, 0, 0, 0, self.ID, frameNumber + 20, StandardOnHit, StandardUpdate, M_ExplosionDraw, self.Pen, self.Brush));
	}

	static bool MissileUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		Vector2 newVel;
		float theta = 0.2f * ((((frameNumber + (int)abs(self.Position.y * 10)) % (((int)abs(self.Position.x) ^ self.ID) + 1)) % 3) - 1);
		newVel.x = self.Velocity.Dot({ cosf(theta), -sinf(theta) });
		newVel.y = self.Velocity.Dot({ sinf(theta), cosf(theta) });
		self.Velocity = newVel;
		if (frameNumber >= self.LifeSpan) {
			parent.Projectiles.Append(Projectile(self.Position, Vector2::Zero, 0, Vector2::Zero, 0, 0, 0, 0, self.ID, frameNumber + 20, StandardOnHit, StandardUpdate, M_ExplosionDraw, self.Pen, self.Brush));
			return true;
		}
		else {
			return false;
		}
	}

	static void MissileDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Pen);
		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		POINT points[5];
		points[0] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * 0.3f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * -0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * -1.0f + perp * -0.3f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + dir * -1.0f + perp * 0.3f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * 0.3f, Scale);

		Polyline(hdc, points, 5);

		points[1] = Giraffe::VecToPoint(self.Position + dir * 0.5f + perp * 0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * 0.75f + perp * 0.3f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + dir, Scale);

		PolyBezier(hdc, points, 4);

		points[0] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * -0.3f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * 0.5f + perp * -0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * 0.75f + perp * -0.3f, Scale);

		PolyBezier(hdc, points, 4);
	}

	static void M_ExplosionDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Pen);
		POINT points[17];
		Vector2 up = { 0, 1.0f };
		Vector2 right = { 1.0f, 0 };
		points[0] = Giraffe::VecToPoint(self.Position + up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 0.25f * up + 0.125f * right, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.5f * up + 0.5f * right, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.125f * up + 0.25f * right, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + right, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + -0.125f * up + 0.25f * right, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + -0.5f * up + 0.5f * right, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + -0.25f * up + 0.125f * right, Scale);
		points[8] = Giraffe::VecToPoint(self.Position + -up, Scale);
		points[9] = Giraffe::VecToPoint(self.Position + -0.25f * up + -0.125f * right, Scale);
		points[10] = Giraffe::VecToPoint(self.Position + -0.5f * up + -0.5f * right, Scale);
		points[11] = Giraffe::VecToPoint(self.Position + -0.125f * up + -0.25f * right, Scale);
		points[12] = Giraffe::VecToPoint(self.Position + -right, Scale);
		points[13] = Giraffe::VecToPoint(self.Position + 0.125f * up + -0.25f * right, Scale);
		points[14] = Giraffe::VecToPoint(self.Position + 0.5f * up + -0.5f * right, Scale);
		points[15] = Giraffe::VecToPoint(self.Position + 0.25f * up + -0.125f * right, Scale);
		points[16] = Giraffe::VecToPoint(self.Position + up, Scale);

		Polyline(hdc, points, 17);
	}

	static void LanceDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Pen);
		SelectObject(hdc, self.Brush);
		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		POINT points[7];
		points[0] = Giraffe::VecToPoint(self.Position, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 3 * dir, Scale);
		Polyline(hdc, points, 2);

		Ellipse(hdc, Scale.x * (self.Position.x + dir.x * 0.2f - 0.2f), Scale.y * (self.Position.y + dir.y * 0.2f - 0.2f), Scale.x * (self.Position.x + dir.x * 0.2f + 0.2f), Scale.y * (self.Position.y + dir.y * 0.2f + 0.2f));
		Ellipse(hdc, Scale.x * (self.Position.x + dir.x * 0.6f - 0.2f), Scale.y * (self.Position.y + dir.y * 0.6f - 0.2f), Scale.x * (self.Position.x + dir.x * 0.6f + 0.2f), Scale.y * (self.Position.y + dir.y * 0.6f + 0.2f));
		Ellipse(hdc, Scale.x * (self.Position.x + dir.x - 0.2f), Scale.y * (self.Position.y + dir.y - 0.2f), Scale.x * (self.Position.x + dir.x + 0.2f), Scale.y * (self.Position.y + dir.y + 0.2f));

		points[0] = Giraffe::VecToPoint(self.Position + dir, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 1.2f * dir + 0.1f * perp, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 1.4f * dir + 0.3f * perp, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 2.1f * dir + 0.3f * perp, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + 1.9f * dir + 0.2f * perp, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + 1.2f * dir + 0.2f * perp, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + dir, Scale);
		Polyline(hdc, points, 7);

		points[0] = Giraffe::VecToPoint(self.Position + dir, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 1.2f * dir - 0.1f * perp, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 1.4f * dir - 0.3f * perp, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 2.1f * dir - 0.3f * perp, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + 1.9f * dir - 0.2f * perp, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + 1.2f * dir - 0.2f * perp, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + dir, Scale);
		Polyline(hdc, points, 7);
	}

	static void SmallLaserDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Pen);
		POINT points[2];
		points[0] = Giraffe::VecToPoint(self.Position - self.Velocity * 0.5f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + self.Velocity * 0.5f, Scale);
		Polyline(hdc, points, 2);
	}

	static void BigLaserDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale) {
		SelectObject(hdc, self.Pen);

		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		POINT points[2];
		points[0] = Giraffe::VecToPoint(self.Position - dir * 3.0f + perp * 0.5f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * 3.0f + perp * 0.5f, Scale);
		Polyline(hdc, points, 2);
		points[0] = Giraffe::VecToPoint(self.Position - dir * 3.0f - perp * 0.5f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * 3.0f - perp * 0.5f, Scale);
		Polyline(hdc, points, 2);
	}
};

#endif // !_ROBOTPROJFUNCS_H_
