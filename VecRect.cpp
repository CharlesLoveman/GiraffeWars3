#include "VecRect.h"

RECT VecRect::toRect(Vector2 Scale)
{
	return { (int)(left * Scale.x), (int)(top * Scale.y), (int)(right * Scale.x), (int)(bottom * Scale.y) };;
}
