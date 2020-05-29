#ifndef _ROBOTPROJFUNCS_H_
#define _ROBOTPROJFUNCS_H_

#include "Giraffe.h"

struct RobotProjFuncs {

	static void GrabberOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		if (collided != nullptr) {
			if (collided->GrabHit(self, parent.Facing, frameNumber)) {
				parent.State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
				parent.State |= STATE_GRABBING;
				parent.TechDelay = frameNumber + 30;
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
};

#endif // !_ROBOTPROJFUNCS_H_
