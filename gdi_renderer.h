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
	void DrawGiraffe(HDC hdc, int which, GameState& GameState);
	void DrawStage(HDC hdc, Stage stage);
	void DrawConnectState(HDC hdc, Giraffe& giraffe, PlayerConnectionInfo& info);
	void DrawGiraffeInfo(HDC hdc, Giraffe& giraffe, int i);
	void CreateGDIFont(HDC hdc);

	Vec2 Scale;
	HFONT _font;
	HWND _hwnd;
	RECT _rc;
	HGLRC _hrc;
	char _status[1024];
	COLORREF _giraffeColours[4];
	HBRUSH _redBrush;
	HBRUSH _stageBrush;
};
#endif
