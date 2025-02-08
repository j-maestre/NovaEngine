#define DIRECTX11

#ifdef DIRECTX11
#include <stdio.h>
#include "windows.h"
#include "windowsx.h"
#include <string>


struct WindowInfo {
	HWND window_handle;
	WNDCLASSEX window_info;
};

struct WindowProperties {
	std::wstring name;
	HBRUSH clear_color;
	HINSTANCE hInstance;
	unsigned int width;
	unsigned int height;
	unsigned int pos_x;
	unsigned int pos_y;
	int nCmdShow;
	bool fullscreen = false;
};

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#endif