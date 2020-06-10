#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

#include "renderer.h"

//Uses GDI to render the game state


class GDIRenderer : public Renderer {
public:
	GDIRenderer(HWND hwnd);
	~GDIRenderer();

	virtual void Draw(GameState& gs, NonGameState& ngs);
	virtual void SetStatusText(const char* text);

protected:
	void RenderChecksum(HDC hdc, int y, NonGameState::ChecksumInfo& info);

	void DrawGameLoop(GameState& gs, NonGameState& ngs, HDC hdc);
	void DrawCharSelect(GameState& gs, NonGameState& ngs, HDC hdc);

	void DrawStage(HDC hdc, Stage stage);
	void DrawConnectState(HDC hdc, Giraffe& giraffe, PlayerConnectionInfo& info);
	void DrawGiraffeInfo(HDC hdc, Giraffe& giraffe, int i);
	void CreateGDIFont(HDC hdc);

	void DrawNormIcon(HDC hdc, Vector2 position, Vector2 scale);
	void DrawPoshIcon(HDC hdc, Vector2 position, Vector2 scale);
	void DrawCoolIcon(HDC hdc, Vector2 position, Vector2 scale);
	void DrawRobotIcon(HDC hdc, Vector2 position, Vector2 scale);

	Vector2 Scale;
	HFONT _font;
	HWND _hwnd;
	RECT _rc;
	HGLRC _hrc;
	char _status[1024];
	COLORREF _giraffeColours[GGPO_MAX_PLAYERS];
	HPEN _giraffePens[GGPO_MAX_PLAYERS];
	HBRUSH _redBrush;
	HBRUSH _stageBrush;
	HBRUSH _blackBrush;
	HPEN SelectedPen;
};
#endif
