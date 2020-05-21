#include "Stage.h"

constexpr float EPSILON = 0.00001f;

bool Stage::Intersects(Vec2 pos, Collider col, bool down, bool falling, int& direction, float& offset)
{
	Vec2 position = pos + col.Position;

	if (falling) {
		for (int p = 0; p < NumPlats; ++p) {
			if (position.y < Plats[p].top && Plats[p].top - position.y - EPSILON < col.Radius && position.x < Plats[p].right && position.x > Plats[p].left) {
				if (down) {
					return false;
				}
				else {
					direction = 0;
					offset = Plats[p].top - col.Radius - position.y;
					return true;
				}
			}
		}
	}

	if (position.x + col.Radius < Box.left || position.x - col.Radius > Box.right || position.y + col.Radius < Box.top || position.y - col.Radius > Box.bottom) {
		return false;
	}

	if (position.y < Box.top && position.x + col.Radius > Box.left && position.x - col.Radius < Box.right) {
		offset = Box.top - col.Radius - position.y;
		direction = 0;
	}
	else if (position.x < Box.left) {
		direction = 2;
		offset = Box.left - col.Radius - position.x;
	}
	else if (position.x > Box.right) {
		direction = 3;
		offset = Box.right + col.Radius - position.x;
	}
	else if (position.y > Box.bottom) {
		direction = 1;
		offset = Box.bottom + col.Radius - position.y;
	}
	else {
		//Just send them up if they are in the middle
		offset = Box.top - col.Radius - position.y;
		direction = 0;
	}

	return true;
}
