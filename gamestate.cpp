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
	float stageleft = 10.0f;
	float stageheight = 5.0f;
	float stagetop = 30.0f;


	//int counter = 0;

	//giraffes[0] = new CoolGiraffe(Vector2(stageleft + stagewidth * 1.0f / (2.0f * _num_giraffes), 20), MoveSets[2], _giraffeColours[0]);
	//giraffes[0] = new PoshGiraffe(Vector2(stageleft + stagewidth * 1.0f / (2.0f * _num_giraffes), 20), MoveSets[3], _giraffeColours[0]);
	//giraffes[1] = new RobotGiraffe(Vector2(stageleft + stagewidth * 3.0f / (2.0f * _num_giraffes), 20), MoveSets[1], _giraffeColours[1]);
	//giraffes[1] = new NormGiraffe(Vector2(stageleft + stagewidth * 3.0f / (2.0f * _num_giraffes), 20), MoveSets[0], _giraffeColours[1]);
	//counter++;

	stage.Box = { stageleft, stagetop, stageleft + stagewidth, stagetop + stageheight };
	stage.Platforms = { {(float)(stageleft + stagewidth / 2.0f - stagewidth / 10.0f), (float)(stagetop - 2 * stageheight - 0.1f), (float)(stageleft + stagewidth / 2.0f + stagewidth / 10.0f), (float)(stagetop - 2 * stageheight + 0.1f)}, {(float)(stageleft + stagewidth / 4.0f - stagewidth / 10.0f), (float)(stagetop - stageheight - 0.1f), (float)(stageleft + stagewidth / 4.0f + stagewidth / 10.0f), (float)(stagetop - stageheight + 0.1f)}, {(float)(stageleft + 3 * stagewidth / 4.0f - stagewidth / 10.0f), (float)(stagetop - stageheight - 0.1f), (float)(stageleft + 3 * stagewidth / 4.0f + stagewidth / 10.0f), (float)(stagetop - stageheight + 0.1f)} };
	stage.Ledges = { { { {stageleft, stagetop + 0.5f}, 0.5f}, false, true}, { {{stageleft + stagewidth, stagetop + 0.5f}, 0.5f}, false, false } };

	CreateGiraffes();

	InflateRect(&_bounds, -8, -8);
}




void GameState::Update(int inputs[], int disconnect_flags)
{
	++_framenumber;

	for (int i = 0; i < _num_giraffes; ++i) {
		if (!(disconnect_flags & (1 << i))) {
			(*giraffes[i]).Update(giraffes, _num_giraffes, i, inputs[i], _framenumber, stage);
		}
	}

	for (int i = 0; i < _num_giraffes; ++i) {
		(*giraffes[i]).Move(stage, _framenumber, giraffes, lines);
	}

	for (int i = lines.size() - 1; i > 0; --i) {
		if (lines[i].Update(_framenumber)) {
			lines.erase(lines.begin() + i);
		}
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
