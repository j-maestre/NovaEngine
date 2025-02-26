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

struct Color {
	float rgba[4];
};

struct WindowInfo {
	HWND window_handle;
	WNDCLASSEX window_info;
	ID3D11RenderTargetView* backbuffer;
};

struct WindowProperties {
	std::wstring name;
	Color clear_color;
	HINSTANCE hInstance;
	unsigned int width;
	unsigned int height;
	unsigned int pos_x;
	unsigned int pos_y;
	int nCmdShow;
	bool fullscreen = false;
};

struct EngineProps {

	// Swap chain props
	DXGI_SWAP_CHAIN_DESC scd;
	IDXGISwapChain* swapChain;
	ID3D11Device* deviceInterface;					// D3D11 device interface
	ID3D11DeviceContext* inmediateDeviceContext;	// D3D11 device context

	D3D_DRIVER_TYPE  driverType;
	uint32 flags = 0;
	D3D_FEATURE_LEVEL feature_level;


};


// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, bool bMenu);


#endif