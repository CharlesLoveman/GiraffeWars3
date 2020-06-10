#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "giraffewar.h"
#include "gamestate.h"
#include "MoveSet.h"
#include "NormGiraffe.h"
#include "PoshGiraffe.h"
#include "SimpleMath.h"


using namespace DirectX::SimpleMath;


extern GGPOSession* ggpo;


//Initialise the game state
void GameState::Init(HWND hwnd, int num_players) {
	int w, h;

	GetClientRect(hwnd, &_bounds);
	InflateRect(&_bounds, -8, -8);

	w = _bounds.right - _bounds.left;
	h = _bounds.bottom - _bounds.top;

	_framenumber = 0;

	_num_giraffes = num_players;

	float stagewidth = 30.0f;
	float stageleft = 12.0f;
	float stageheight = 5.0f;
	float stagetop = 30.0f;


	state = 2;

	stage.Box = { stageleft, stagetop, stageleft + stagewidth, stagetop + stageheight };
	stage.Platforms = { {(float)(stageleft + stagewidth / 2.0f - stagewidth / 10.0f), (float)(stagetop - 2 * stageheight - 0.1f), (float)(stageleft + stagewidth / 2.0f + stagewidth / 10.0f), (float)(stagetop - 2 * stageheight + 0.1f)}, {(float)(stageleft + stagewidth / 4.0f - stagewidth / 10.0f), (float)(stagetop - stageheight - 0.1f), (float)(stageleft + stagewidth / 4.0f + stagewidth / 10.0f), (float)(stagetop - stageheight + 0.1f)}, {(float)(stageleft + 3 * stagewidth / 4.0f - stagewidth / 10.0f), (float)(stagetop - stageheight - 0.1f), (float)(stageleft + 3 * stagewidth / 4.0f + stagewidth / 10.0f), (float)(stagetop - stageheight + 0.1f)} };
	stage.Ledges = { { { {stageleft, stagetop + 0.5f}, 0.5f}, false, true}, { {{stageleft + stagewidth, stagetop + 0.5f}, 0.5f}, false, false } };


	InflateRect(&_bounds, -8, -8);
}




void GameState::Update(int inputs[], int disconnect_flags)
{
	++_framenumber;

	//State 0 is the main game
	switch (state) {
	case 0:
	{
		int alive = _num_giraffes;
		for (int i = 0; i < _num_giraffes; ++i) {
			if (!(disconnect_flags & (1 << i)) && giraffes[i]->Stocks > 0) {
				giraffes[i]->Update(giraffes, _num_giraffes, i, inputs[i], _framenumber, stage);
			}
			else {
				--alive;
			}
		}

		if (alive <= 1) {
			for (int i = 0; i < _num_giraffes; ++i) {
				giraffes[i]->Position = { (stage.Box.left + stage.Box.right) / 2.0f, 30 };
				selectDelay[i] = _framenumber + 30;
			}
			state = 3;
			break;
		}

		for (int i = 0; i < _num_giraffes; ++i) {
			giraffes[i]->Move(stage, _framenumber, giraffes, lines);
		}

		for (int i = lines.size() - 1; i > 0; --i) {
			if (lines[i].Update(_framenumber)) {
				lines.erase(lines.begin() + i);
			}
		}
		break;
	}
		
	//State 1 is the character selection
	case 1:
	{
		if (ParseCharSelectInputs(inputs)) {
			CreateGiraffes();
			state = 0;
		}
		break;
	}
	//State 2 is the title screen
	case 2:
	{
		int Finished = 0;
		for (int i = 0; i < _num_giraffes; ++i) {
			Finished += inputs[i];
		}
		if (Finished) {
			state = 1;
			for (int i = 0; i < _num_giraffes; ++i) {
				selectDelay[i] = _framenumber + 30;
			}
		}
		break;

	}
	//State 3 is the win screen
	case 3:
		int Finished = 0;
		for (int i = 0; i < _num_giraffes; ++i) {
			if (_framenumber >= selectDelay[i]) {
				Finished += inputs[i];
			}
		}
		if (Finished) {
			state = 1;
			for (int i = 0; i < _num_giraffes; ++i) {
				delete giraffes[i];
				selected[i] = false;
				selectDelay[i] = _framenumber + 30;
			}
		}
		break;
	}

	
}

void GameState::CreateGiraffes()
{
	COLORREF _giraffeColours[4];
	_giraffeColours[0] = RGB(255, 0, 0);
	_giraffeColours[1] = RGB(0, 255, 0);
	_giraffeColours[2] = RGB(0, 0, 255);

	_giraffeColours[3] = RGB(255, 255, 0);
	for (int i = 0; i < _num_giraffes; ++i) {
		switch (selectors[i])
		{
		case 1:
			giraffes[i] = new PoshGiraffe(Vector2(stage.Box.left + (stage.Box.right - stage.Box.left) * (2 * i + 1) / (2.0f * _num_giraffes), 20), _giraffeColours[i]);
			break;
		case 2:
			giraffes[i] = new CoolGiraffe(Vector2(stage.Box.left + (stage.Box.right - stage.Box.left) * (2 * i + 1) / (2.0f * _num_giraffes), 20), _giraffeColours[i]);
			break;
		case 3:
			giraffes[i] = new RobotGiraffe(Vector2(stage.Box.left + (stage.Box.right - stage.Box.left) * (2 * i + 1) / (2.0f * _num_giraffes), 20), _giraffeColours[i]);
			break;
		default:
			giraffes[i] = new NormGiraffe(Vector2(stage.Box.left + (stage.Box.right - stage.Box.left) * (2 * i + 1) / (2.0f * _num_giraffes), 20), _giraffeColours[i]);
			break;
		}
	}
}

bool GameState::ParseCharSelectInputs(int inputs[])
{
	bool Finished = true;
	for (int i = 0; i < _num_giraffes; ++i) {
		if (_framenumber >= selectDelay[i]) {
			if (inputs[i] & INPUT_WEAK && !selected[i]) {
				selected[i] = true;
				selectDelay[i] = _framenumber + 10;
			}
			else if (inputs[i] & INPUT_HEAVY && selected[i]) {
				selected[i] = false;
				selectDelay[i] = _framenumber + 10;
			}

			if (!selected[i]) {
				if (inputs[i] & INPUT_LEFT) {
					selectors[i] = (selectors[i] - 1) % 4;
					selectDelay[i] = _framenumber + 10;
				}
				else if (inputs[i] & INPUT_RIGHT) {
					selectors[i] = (selectors[i] + 1) % 4;
					selectDelay[i] = _framenumber + 10;
				}
			}
		}
		Finished &= selected[i];

	}
	return Finished;
}
