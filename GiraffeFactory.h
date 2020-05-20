#ifndef _GIRAFFEFACTORY_H_
#define _GIRAFFEFACTORY_H_

#include "MoveSet.h"
#include "GiraffeRenderer.h"
#include <Windows.h>


typedef void (*fptr)(HDC, std::array<Vec2, NUM_POINTS>, Vec2, Vec2, Vec2);


class GiraffeFactory{
public:
	static fptr GetDrawFunc(int i);

private:
	GiraffeFactory();
};

#endif // !_NORMGIRAFFE_H_

