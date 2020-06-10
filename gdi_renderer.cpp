#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "giraffewar.h"
#include "gdi_renderer.h"
#include <string>
#include <bitset>


constexpr int PROGRESS_BAR_WIDTH = 100;
constexpr int PROGRESS_BAR_TOP_OFFSET = 22;
constexpr int PROGRESS_BAR_HEIGHT = 8;
constexpr int PROGRESS_TEXT_OFFSET = PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT + 4;

GDIRenderer::GDIRenderer(HWND hwnd) :
	_hwnd(hwnd)
{
	HDC hdc = GetDC(_hwnd);
	*_status = '\0';
	GetClientRect(hwnd, &_rc);
	CreateGDIFont(hdc);
	ReleaseDC(_hwnd, hdc);

	Scale = { 1.0f / 54.0f * (_rc.right - _rc.left), 1.0f / 48.0f * (_rc.bottom - _rc.top) };

	_giraffeColours[0] = RGB(255, 0, 0);
	_giraffeColours[1] = RGB(0, 255, 0);
	_giraffeColours[2] = RGB(0, 0, 255);
	_giraffeColours[3] = RGB(255, 255, 0);

	SelectedPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i) {
		_giraffePens[i] = CreatePen(PS_SOLID, 1, _giraffeColours[i]);
	}

	_redBrush = CreateSolidBrush(RGB(255, 0, 0));
	_stageBrush = CreateSolidBrush(RGB(127, 127, 127));
	_blackBrush = CreateSolidBrush(0);
}

GDIRenderer::~GDIRenderer() 
{
	DeleteObject(_font);
}

void GDIRenderer::Draw(GameState& gs, NonGameState& ngs)
{
	HDC hdc = GetDC(_hwnd);

	FillRect(hdc, &_rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	FrameRect(hdc, &gs._bounds, (HBRUSH)GetStockObject(WHITE_BRUSH));

	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, _font);

	switch (gs.state) {
	case 1:
		DrawCharSelect(gs, ngs, hdc);
		break;
	default:
		DrawGameLoop(gs, ngs, hdc);
		break;
	}

	

	SetTextAlign(hdc, TA_BOTTOM | TA_CENTER);
	TextOutA(hdc, (_rc.left + _rc.right) / 2, _rc.bottom - 32, _status, (int)strlen(_status));

	SetTextColor(hdc, RGB(192, 192, 192));
	RenderChecksum(hdc, 40, ngs.periodic);
	SetTextColor(hdc, RGB(128, 128, 128));
	RenderChecksum(hdc, 56, ngs.now);

	//SwapBuffers(hdc);//????
	ReleaseDC(_hwnd, hdc);
}

void GDIRenderer::RenderChecksum(HDC hdc, int y, NonGameState::ChecksumInfo& info) 
{
	char checksum[128];
	sprintf_s(checksum, ARRAYSIZE(checksum), "Frame: %04d  Checksum: %08x", info.framenumber, info.checksum);
	TextOutA(hdc, (_rc.left + _rc.right) / 2, _rc.top + y, checksum, (int)strlen(checksum));
}

void GDIRenderer::DrawGameLoop(GameState& gs, NonGameState& ngs, HDC hdc)
{
	for (int i = 0; i < gs.lines.size(); ++i) {
		gs.lines[i].Draw(hdc, Scale, gs._framenumber);
	}

	for (int i = 0; i < gs._num_giraffes; ++i) {
		SetTextColor(hdc, _giraffeColours[i]);
		gs.giraffes[i]->Draw(hdc, Scale, gs._framenumber);
		DrawConnectState(hdc, *gs.giraffes[i], ngs.players[i]);
		DrawGiraffeInfo(hdc, *gs.giraffes[i], i);
	}

	DrawStage(hdc, gs.stage);
}

void GDIRenderer::DrawCharSelect(GameState& gs, NonGameState& ngs, HDC hdc)
{
	float width = _rc.right - _rc.left;
	float height = _rc.bottom - _rc.top;
	
	DrawNormIcon(hdc, { width / 5.0f, height / 2.0f }, { width / 5.0f, height / 3.0f });
	DrawPoshIcon(hdc, { (2 * width) / 5.0f, height / 2.0f }, { width / 5.0f, height / 3.0f });
	DrawCoolIcon(hdc, { (3 * width) / 5.0f, height / 2.0f }, { width / 5.0f, height / 3.0f });
	DrawRobotIcon(hdc, { (4 * width) / 5.0f, height / 2.0f }, { width / 5.0f, height / 3.0f });

	for (int i = 0; i < gs._num_giraffes; ++i) {
		SetTextColor(hdc, _giraffeColours[i]);
		if (gs.selected[i]) {
			SelectObject(hdc, SelectedPen);
		}
		else {
			SelectObject(hdc, _giraffePens[i]);
		}

		POINT points[5];
		points[0] = { (int)(width / 5.0f * (gs.selectors[i] + 0.5f) + i), (int)(height / 3.0f + i) };
		points[1] = { (int)(width / 5.0f * (gs.selectors[i] + 1.5f) - i), (int)(height / 3.0f + i) };
		points[2] = { (int)(width / 5.0f * (gs.selectors[i] + 1.5f) - i), (int)((2 * height) / 3.0f - i) };
		points[3] = { (int)(width / 5.0f * (gs.selectors[i] + 0.5f) + i), (int)((2 * height) / 3.0f - i) };
		points[4] = { (int)(width / 5.0f * (gs.selectors[i] + 0.5f) + i), (int)(height / 3.0f + i) };
		Polyline(hdc, points, 5);
	}
}

void GDIRenderer::SetStatusText(const char* text)
{
	strcpy_s(_status, text);
}

void GDIRenderer::DrawStage(HDC hdc, Stage stage)
{
	stage.Draw(hdc, Scale, _stageBrush);
	SetTextAlign(hdc, TA_TOP | TA_CENTER);
}

void GDIRenderer::DrawConnectState(HDC hdc, Giraffe& giraffe, PlayerConnectionInfo& info) 
{
	char status[64];
	static const char* statusStrings[] = {
		"Connecting...",
		"Synchronizing...",
		"",
		"Disconnected.",
	};
	int progress = -1;

	*status = '\0';
	switch (info.state) {
	case Connecting:
		sprintf_s(status, ARRAYSIZE(status), (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Connecting...");
		break;

	case Synchronizing:
		progress = info.connect_progress;
		sprintf_s(status, ARRAYSIZE(status), (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Synchronizing...");
		break;

	case Disconnected:
		sprintf_s(status, ARRAYSIZE(status), "Disconnected");
		break;

	case Disconnecting:
		sprintf_s(status, ARRAYSIZE(status), "Waiting for player...");
		progress = (timeGetTime() - info.disconnect_start) * 100 / info.disconnect_timeout;
		break;
	}

	if (*status) {
		SetTextAlign(hdc, TA_TOP | TA_CENTER);
		TextOutA(hdc, (int)(giraffe.Position.x * Scale.x), (int)(giraffe.Position.y * Scale.y) + PROGRESS_TEXT_OFFSET, status, (int)strlen(status));
	}


	if (progress >= 0) {
		HBRUSH bar = (HBRUSH)(info.state == Synchronizing ? GetStockObject(WHITE_BRUSH) : _redBrush);
		RECT rc = { (LONG)(Scale.x * giraffe.Position.x - (PROGRESS_BAR_WIDTH / 2)),
					(LONG)(Scale.y * giraffe.Position.y + PROGRESS_BAR_TOP_OFFSET),
					(LONG)(Scale.x * giraffe.Position.x + (PROGRESS_BAR_WIDTH / 2)),
					(LONG)(Scale.y * giraffe.Position.y + PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT) };

		FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		rc.right = rc.left + min(100, progress) * PROGRESS_BAR_WIDTH / 100;
		InflateRect(&rc, -1, -1);
		FillRect(hdc, &rc, bar);
	}
}

void GDIRenderer::DrawGiraffeInfo(HDC hdc, Giraffe& giraffe, int i)
{
	/*char msg[64];
	*msg = '\0';
	std::bitset<32> bits(giraffe.State);
	sprintf_s(msg, ARRAYSIZE(msg), bits.to_string().c_str());*/
	SetTextAlign(hdc, TA_TOP | TA_CENTER);
	//TextOutA(hdc, 150 + 250 * i, 0, msg, (int)strlen(msg));

	TextOutA(hdc, 150 + 250 * i, 0, std::to_string(giraffe.Knockback * 10).c_str(), 8);
}

void GDIRenderer::CreateGDIFont(HDC)
{
	_font = CreateFontW(
		-12,
		0,
		0,
		0,
		0,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		FF_DONTCARE | DEFAULT_PITCH,
		L"Tahoma");
}

void GDIRenderer::DrawNormIcon(HDC hdc, Vector2 position, Vector2 scale)
{
	SelectObject(hdc, SelectedPen);
	POINT points[6];
	Vector2 right = { scale.x, 0 };
	Vector2 up = { 0, -scale.y };
	Vector2 _scale = { 1,1 };

	points[0] = Giraffe::VecToPoint(position - 0.4f * right - 0.15f * up, _scale);
	points[1] = Giraffe::VecToPoint(position - 0.4f * right + 0.15f * up, _scale);
	points[2] = Giraffe::VecToPoint(position + 0.2f * right + 0.15f * up, _scale);
	points[3] = Giraffe::VecToPoint(position + 0.4f * right, _scale);
	points[4] = Giraffe::VecToPoint(position + 0.2f * right - 0.15f * up, _scale);
	points[5] = Giraffe::VecToPoint(position - 0.4f * right - 0.15f * up, _scale);

	Polyline(hdc, points, 6);
}

void GDIRenderer::DrawPoshIcon(HDC hdc, Vector2 position, Vector2 scale)
{
	SelectObject(hdc, SelectedPen);
	POINT points[6];
	Vector2 right = { scale.x, 0 };
	Vector2 up = { 0, -scale.y };
	Vector2 _scale = { 1,1 };

	points[0] = Giraffe::VecToPoint(position - 0.15f * right - 0.3f * up, _scale);
	points[1] = Giraffe::VecToPoint(position - 0.15f * right - 0.1f * up, _scale);
	points[2] = Giraffe::VecToPoint(position + 0.15f * right - 0.1f * up, _scale);
	points[3] = Giraffe::VecToPoint(position + 0.25f * right - 0.2f * up, _scale);
	points[4] = Giraffe::VecToPoint(position + 0.15f * right - 0.3f * up, _scale);
	points[5] = Giraffe::VecToPoint(position - 0.15f * right - 0.3f * up, _scale);

	Polyline(hdc, points, 6);

	points[0] = Giraffe::VecToPoint(position - 0.4f * right - 0.1f * up, _scale);
	points[1] = Giraffe::VecToPoint(position - 0.15f * right - 0.1f * up, _scale);
	points[2] = Giraffe::VecToPoint(position - 0.15f * right + 0.4f * up, _scale);
	points[3] = Giraffe::VecToPoint(position + 0.15f * right + 0.4f * up, _scale);
	points[4] = Giraffe::VecToPoint(position + 0.15f * right - 0.1f * up, _scale);
	points[5] = Giraffe::VecToPoint(position + 0.4f * right - 0.1f * up, _scale);

	Polyline(hdc, points, 6);

	SelectObject(hdc, _blackBrush);
	Vector2 MonoclePos = position + 0.1f * right - 0.2f * up;
	float radius = scale.x / 20.0f;
	Ellipse(hdc, (int)(MonoclePos.x - radius), (int)(MonoclePos.y - radius), (int)(MonoclePos.x + radius), (int)(MonoclePos.y + radius));

	points[0] = Giraffe::VecToPoint(MonoclePos - Vector2(radius, 0), _scale);
	points[1] = Giraffe::VecToPoint(MonoclePos - Vector2(radius, -2 * radius), _scale);

	Polyline(hdc, points, 2);
}

void GDIRenderer::DrawCoolIcon(HDC hdc, Vector2 position, Vector2 scale)
{
	SelectObject(hdc, SelectedPen);
	POINT points[21];
	Vector2 right = { scale.x, 0 };
	Vector2 up = { 0, -scale.y };
	Vector2 _scale = { 1,1 };

	points[0] = Giraffe::VecToPoint(position - 0.1f * right - 0.15f * up, _scale);
	points[1] = Giraffe::VecToPoint(position - 0.1f * right + 0.15f * up, _scale);
	points[2] = Giraffe::VecToPoint(position + 0.3f * right + 0.15f * up, _scale);
	points[3] = Giraffe::VecToPoint(position + 0.4f * right, _scale);
	points[4] = Giraffe::VecToPoint(position + 0.3f * right - 0.15f * up, _scale);
	points[5] = Giraffe::VecToPoint(position - 0.1f * right - 0.15f * up, _scale);

	Polyline(hdc, points, 6);

	for (int i = 0; i < 20; ++i) {
		points[i] = Giraffe::VecToPoint(position -0.1f * right + 0.15f * up + (-i / 50.0f * right) + 0.1f * sinf(i / (3.14159f)) * up, _scale);
	}
	Polyline(hdc, points, 20);

	for (int i = 20; i >= 0; --i) {
		points[i] = points[i-1];
		points[i].y -= 0.05f * up.y;
	}
	points[0] = Giraffe::VecToPoint(position + 0.3333333f * right + 0.1f * up, _scale);
	Polyline(hdc, points, 20);
}

void GDIRenderer::DrawRobotIcon(HDC hdc, Vector2 position, Vector2 scale)
{
	SelectObject(hdc, SelectedPen);
	Vector2 right = { scale.x, 0 };
	Vector2 up = { 0, -scale.y };
	Vector2 _scale = { 1,1 };

	POINT points[5];
	points[0] = Giraffe::VecToPoint(position - 0.3f * right - 0.15f * up, _scale);
	points[1] = Giraffe::VecToPoint(position - 0.3f * right + 0.15f * up, _scale);
	points[2] = Giraffe::VecToPoint(position + 0.3f * right + 0.15f * up, _scale);
	points[3] = Giraffe::VecToPoint(position + 0.3f * right - 0.15f * up, _scale);
	points[4] = Giraffe::VecToPoint(position - 0.3f * right - 0.15f * up, _scale);

	Polyline(hdc, points, 5);

	SelectObject(hdc, _redBrush);
	Vector2 EyePos = position + 0.15f * right;
	float radius = scale.x / 25.0f;
	Ellipse(hdc, (int)(EyePos.x - radius), (int)(EyePos.y - radius), (int)(EyePos.x + radius), (int)(EyePos.y + radius));
}
