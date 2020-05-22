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

	for (int i = 0; i < 4; ++i) {
		_giraffePens[i] = CreatePen(PS_SOLID, 1, _giraffeColours[i]);
	}

	_intangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	_redBrush = CreateSolidBrush(RGB(255, 0, 0));
	_stageBrush = CreateSolidBrush(RGB(127, 127, 127));
	_shieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
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

	for (int i = 0; i < gs._num_giraffes; ++i) {
		SetTextColor(hdc, _giraffeColours[i]);
		//SelectObject(hdc, _giraffePens[i]);
		gs.giraffes[i]->Draw(hdc, Scale, _shieldBrush, _giraffePens[i], _intangiblePen);
		DrawConnectState(hdc, *gs.giraffes[i], ngs.players[i]);
		DrawGiraffeInfo(hdc, *gs.giraffes[i], i);
	}

	DrawStage(hdc, gs.stage);

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

void GDIRenderer::SetStatusText(const char* text)
{
	strcpy_s(_status, text);
}

void GDIRenderer::DrawGiraffe(HDC hdc, int which, GameState& gs)
{
	Giraffe* giraffe = gs.giraffes[which];
	
	//Ellipse(hdc, (int)(giraffe->Position.x - 10), (int)(giraffe->Position.y - 10), (int)(giraffe->Position.x + 10), (int)(giraffe->Position.y + 10));
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
	char msg[64];
	*msg = '\0';
	std::bitset<32> bits(giraffe.State);
	sprintf_s(msg, ARRAYSIZE(msg), bits.to_string().c_str());
	SetTextAlign(hdc, TA_TOP | TA_CENTER);
	TextOutA(hdc, 150 + 250 * i, 0, msg, (int)strlen(msg));

	//TextOutA(hdc, 150 + 250 * i, 0, std::to_string(giraffe.Knockback * 10).c_str(), 8);
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