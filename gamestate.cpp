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