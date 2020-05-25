#ifndef _VECRECT_H_
#define _VECRECT_H_

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;


struct VecRect {
public:
	float left, top, right, bottom;
	RECT toRect(Vector2 Scale);
};

#endif // !_VECRECT_H_
