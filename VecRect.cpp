#include "VecRect.h"

RECT VecRect::ToRect(Vec2 Scale)
{
	return { (int)(left * Scale.x), (int)(top * Scale.y), (int)(right * Scale.x), (int)(bottom * Scale.y) };
}
