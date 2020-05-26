#include <windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#if defined(_DEBUG)
#	include <crtdbg.h>
#endif
#include "giraffewar.h"
#include "Audio.h"
#include <memory>


LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			GiraffeWar_Exit();
		}
		else if (wParam >= VK_F1 && wParam <= VK_F12) {
			GiraffeWar_DisconnectPlayer((int)(wParam - VK_F1));
		}
		return 0;
	case WM_PAINT:
		GiraffeWar_DrawCurrentFrame();
		ValidateRect(hwnd, NULL);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam);
}


HWND CreateMainWindow(HINSTANCE hInstance, int width, int height)
{
	HWND hwnd;
	WNDCLASSEXW wndclass = { 0 };
	RECT rc;
	WCHAR titlebuf[128];

	wsprintfW(titlebuf, L"(pid:%d) GiraffeWar", GetCurrentProcessId());
	wndclass.cbSize = sizeof(wndclass);
	wndclass.lpfnWndProc = MainWindowProc;
	wndclass.lpszClassName = L"vwwnd";
	RegisterClassExW(&wndclass);
	hwnd = CreateWindowW(L"vwwnd", titlebuf, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
	GetClientRect(hwnd, &rc);
	SetWindowPos(hwnd, NULL, 0, 0, width + (width - (rc.right - rc.left)), height + (height - (rc.bottom - rc.top)), SWP_NOMOVE);
	return hwnd;
}


void RunMainLoop(HWND hwnd)
{
	MSG msg = { 0 };
	int start, next, now;

	start = next = now = timeGetTime();
	while (1) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				return;
			}
		}
		now = timeGetTime();
		GiraffeWar_Idle(max(0, next - now - 1));
		if (now >= next) {
			GiraffeWar_RunFrame(hwnd);
			next = now + (1000 / 60);
		}
	}
}


void Syntax()
{
	MessageBoxW(NULL, L"Syntax: GiraffeWar.exe <local port> <num players(2)> ('local' | <remote ip>:<remote port>)*\n", L"Could not start", MB_OK);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	std::ifstream myFile;

	char path[128];
	GetModuleFileNameA(NULL, path, 128);
	std::string filepath = std::string(path);

	myFile.open(filepath.substr(0, filepath.length() - 16) + "Config.txt");
	
	int width, height;
	std::string input[10];
	for (int i = 0; i < 10; ++i) {
		std::getline(myFile, input[i]);
	}
	myFile.close();
	
	width = std::stoi(input[0]);
	height = std::stoi(input[1]);
	
	HWND hwnd = CreateMainWindow(hInstance, width, height);
	int offset = 1, local_player = 0;
	WSADATA wd = { 0 };
	wchar_t wide_ip_buffer[128];
	unsigned int wide_ip_buffer_size = (unsigned int)ARRAYSIZE(wide_ip_buffer);

	WSAStartup(MAKEWORD(2, 2), &wd);
	POINT window_offsets[] = {
		{64,64},
		{740, 64},
		{64, 600},
		{740, 600}
	};


	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		MessageBox(NULL, "Failed to initialise audio", "Error", 0);
		return 1;
	}

#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (__argc < 3) {
		Syntax();
		return 1;
	}

	unsigned short local_port = (unsigned short)_wtoi(__wargv[offset++]);
	int num_players = _wtoi(__wargv[offset++]);
	if (num_players < 0 || __argc < offset + num_players) {
		Syntax();
		return 1;
	}

	if (wcscmp(__wargv[offset], L"spectate") == 0) {
		char host_ip[128];
		unsigned short host_port;
		if (swscanf_s(__wargv[offset + 1], L"%[^:]:%hu", wide_ip_buffer, wide_ip_buffer_size, &host_port) != 2) {
			Syntax();
			return 1;
		}
		wcstombs_s(nullptr, host_ip, ARRAYSIZE(host_ip), wide_ip_buffer, _TRUNCATE);
		GiraffeWar_InitSpectator(hwnd, local_port, num_players, host_ip, host_port);
	}
	else {
		GGPOPlayer players[GGPO_MAX_SPECTATORS + GGPO_MAX_PLAYERS];

		int i;
		for (i = 0; i < num_players; i++) {
			const wchar_t* arg = __wargv[offset++];

			players[i].size = sizeof(players[i]);
			players[i].player_num = i + 1;
			if (!_wcsicmp(arg, L"local")) {
				players[i].type = GGPO_PLAYERTYPE_LOCAL;
				local_player = i;
				continue;
			}

			players[i].type = GGPO_PLAYERTYPE_REMOTE;
			if (swscanf_s(arg, L"%[^:]:%hd", wide_ip_buffer, wide_ip_buffer_size, &players[i].u.remote.port) != 2) {
				Syntax();
				return 1;
			}
			wcstombs_s(nullptr, players[i].u.remote.ip_address, ARRAYSIZE(players[i].u.remote.ip_address), wide_ip_buffer, _TRUNCATE);
		}
		
		int num_spectators = 0;
		while (offset < __argc) {
			players[i].type = GGPO_PLAYERTYPE_SPECTATOR;
			if (swscanf_s(__wargv[offset++], L"%[^:]:%hd", wide_ip_buffer, wide_ip_buffer_size, &players[i].u.remote.port) != 2) {
				Syntax();
				return 1;
			}
			wcstombs_s(nullptr, players[i].u.remote.ip_address, ARRAYSIZE(players[i].u.remote.ip_address), wide_ip_buffer, _TRUNCATE);
			++i;
			++num_spectators;
		}

		if (local_player < sizeof(window_offsets) / sizeof(window_offsets[0])) {
			::SetWindowPos(hwnd, NULL, window_offsets[local_player].x, window_offsets[local_player].y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}


		int inputKeys[8];
		HKL dwhkl = GetKeyboardLayout(0);
		for (int j = 0; j < 8; ++j) {
			if (input[j + 2].length() == 1) {
				inputKeys[j] = 0x00FF & VkKeyScanExA(input[j + 2][0], dwhkl);
			}
			else {
				if (input[j + 2] == "UP") {
					inputKeys[j] = VK_UP;
				}
				else if (input[j + 2] == "LEFT") {
					inputKeys[j] = VK_LEFT;
				}
				else if (input[j + 2] == "DOWN") {
					inputKeys[j] = VK_DOWN;
				}
				else if (input[j + 2] == "RIGHT") {
					inputKeys[j] = VK_RIGHT;
				}
				else if (input[j + 2] == "SPACE") {
					inputKeys[j] = VK_SPACE;
				}
				else {
					MessageBox(NULL, "Error Reading Config File: Unknown Key", input[j + 2].c_str(), MB_OK);
					return 1;
				}
			}
		}

		GiraffeWar_Init(hwnd, local_port, players, num_players, num_spectators, inputKeys);
	}
	RunMainLoop(hwnd);
	GiraffeWar_Exit();
	WSACleanup();
	DestroyWindow(hwnd);
	return 0;
}