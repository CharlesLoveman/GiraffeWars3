#include "Stage.h"

bool Stage::Intersects(Vec2 pos, Collider col, int& direction, float& offset)
{
	Vec2 position = pos + col.Position;
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
