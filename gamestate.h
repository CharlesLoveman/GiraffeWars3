#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_
#include "Giraffe.h"
#include "NormGiraffe.h"
#include "RobotGiraffe.h"
#include "CoolGiraffe.h"
#include "Stage.h"
#include "MoveSet.h"
#include <memory>
#include <array>
#include "Line.h"

//Structure to be serialized at each frame
//Needs to be small

constexpr int MAX_PLAYERS = 4;


struct GameState
{
	void Init(HWND hwnd, int num_players, const std::array<MoveSet*, 4> MoveSets);
	void Update(int inputs[], int disconnect_flags);

	int _framenumber;
	int _num_giraffes;
	RECT _bounds;
	std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes;
	Stage stage;
	std::vector<Line> lines;
};
#endif // !_GAMESTATE_H_

