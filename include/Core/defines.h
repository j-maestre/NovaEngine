#pragma once

#define DIRECTX11

#ifdef DIRECTX11
// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

#include "windows.h"
#include "windowsx.h"

/*** Directx11 include files ***/
#include <d3d11.h>
#include <DirectXMath.h>
/*******************************/


/*** Deprecated ***/
// #include <d3dx11.h>
// #include <d3dx10.h>
/******************/

#include <stdio.h>
#include <string>


typedef unsigned int uint32;



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

BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, bool bMenu);


#endif