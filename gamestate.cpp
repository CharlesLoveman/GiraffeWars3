#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "giraffewar.h"
#include "gamestate.h"
#include "MoveSet.h"
#include "NormGiraffe.h"


extern GGPOSession* ggpo;


//Initialise the game state
void GameState::Init(HWND hwnd, int num_players, const std::array<MoveSet*, 4> MoveSets) {
	int w, h;
	//const MoveSet Moves[4] = { MoveSet(), MoveSet(), MoveSet(), MoveSet() };

	GetClientRect(hwnd, &_bounds);
	InflateRect(&_bounds, -8, -8);

	w = _bounds.right - _bounds.left;
	h = _bounds.bottom - _bounds.top;

	_framenumber = 0;

	_num_giraffes = num_players;

	float stagewidth = 30.0f;
	float stageleft = 10.0f;
	float stageheight = 5.0f;
	float stagetop = 30.0f;

	for (int i = 0; i < _num_giraffes; ++i) {
		normGiraffes.push_back(NormGiraffe(Vec2(stageleft + stagewidth * (2.0f * i + 1.0f) / (2.0f * _num_giraffes), 20), MoveSets[0]));
	}

	for (int i = 0; i < normGiraffes.size(); ++i) {
		giraffes[i] = &normGiraffes[i];
	}

	stage.Box = {
				stageleft,
				stagetop,
				stageleft + stagewidth,
				stagetop + stageheight};

	InflateRect(&_bounds, -8, -8);
}


//void GameState::GetGiraffeAI(int i) {
//	//If you disconnect
//	//Do nothing
//}


//void GameState::ParseGiraffeInputs(int inputs, int i) {
//	Giraffe* giraffe = giraffes + i;
//
//	ggpo_log(ggpo, "parsing paddle %d inputs: %d.\n", i, inputs);
//
//	if (inputs & INPUT_LEFT) {
//		*direction = -1;
//	}
//	else if (inputs & INPUT_RIGHT) {
//		*direction = 1;
//	}
//	else {
//		*direction = 0;
//	}
//}


//void GameState::MovePaddle(int which, int direction) {
//	Paddle* paddle = _paddles + which;
//
//	ggpo_log(ggpo, "calculation of new paddle %d coordinates (direction: %d).\n", which, direction);
//
//	if (direction == 1) {
//		paddle->velocity = { 0, paddle->speed };
//	}
//	else if (direction == -1) {
//		paddle->velocity = { 0, -paddle->speed };
//	}
//
//	//paddle->position.x += paddle->velocity.dx;
//	paddle->position.y += paddle->velocity.dy;
//
//	if ((paddle->position.y - (paddle->height / 2) < _bounds.top) || (paddle ->position.y + (paddle->height / 2) > _bounds.bottom)) {
//		paddle->velocity.dy *= -1;
//		paddle->position.y += paddle->velocity.dy * 2;
//	}
//}


void GameState::Update(int inputs[], int disconnect_flags)
{
	++_framenumber;

	for (int i = 0; i < _num_giraffes; ++i) {
		if (!(disconnect_flags & (1 << i))) {
			(*giraffes[i]).Update(giraffes, _num_giraffes, i, inputs[i], _framenumber);
		}
	}

	for (int i = 0; i < _num_giraffes; ++i) {
		(*giraffes[i]).Move(stage, _framenumber);
	}
}