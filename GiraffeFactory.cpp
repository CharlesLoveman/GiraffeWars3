#include "GiraffeFactory.h"


fptr GiraffeFactory::GetDrawFunc(int i)
{
	switch (i) {
	case 0:
		return GiraffeRenderer::DrawNorm;
	case 1:
		return GiraffeRenderer::DrawCool;
	case 2:
		return GiraffeRenderer::DrawPosh;
	case 3:
		return GiraffeRenderer::DrawRobot;
	}
}
