#ifndef _POSHPROJFUNCS_H_
#define _POSHPROJFUNCS_H_

#include "Giraffe.h"

struct PoshProjFuncs {
	static void StandardOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Do nothing?
	}

	static bool StandardUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}
};
#endif