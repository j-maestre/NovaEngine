#pragma once

#include <vector>

#define DIRECTX11


constexpr unsigned int SCREEN_WIDTH = 1920;
constexpr unsigned int SCREEN_HEIGHT = 1200;


constexpr float kfPi = 3.14159265f;
constexpr float kfDegToRad = kfPi / 180.0f;
constexpr float kfRadToDeg = 180.0f / kfPi;


static inline float degToRad(const float degrees) {
	return degrees * kfDegToRad;
}


static inline float radToDeg(const float radians) {
	return radians * kfRadToDeg;
}


struct Vertex {
	float pos_x, pos_y, pos_z;
	float nrm_x, nrm_y, nrm_z;
	float uv_x, uv_y;
};


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

#define SHADER_PATH_FORWARD "data/shaders/forward";
#define SHADER_PATH_DEFERRED "data/shaders/deferred";

struct ShaderFiles {
	ID3D11VertexShader* VS_directional = nullptr;
	ID3D11PixelShader* PS_directional = nullptr;
};

enum class ShaderType {
	DirectionalLight = 0,
	SpotLight,
	PointLight,
};

enum class ErrorCode {
	OK = 0,
	UNKNOW,
};


typedef unsigned int uint32;

/******* Use for store *******/ 
typedef DirectX::XMFLOAT2 Vec2;
typedef DirectX::XMFLOAT3 Vec3;
typedef DirectX::XMFLOAT4 Vec4;
/*****************************/

typedef DirectX::XMVECTOR FVector;		// Used for calculations
typedef DirectX::CXMVECTOR CVec4;		// Const ref of XMVector

typedef DirectX::XMMATRIX Mat4;

struct CameraConstantBuffer {
	Mat4 view;
	Mat4 projection;
	Mat4 model;
	Vec3 camera_position;
};


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

