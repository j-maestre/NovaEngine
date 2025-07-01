#pragma once

#include <vector>

#define DIRECTX11
#define ENABLE_IMGUI
#define MEASURE_TIME


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






#ifdef DIRECTX11
// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

#include "windows.h"
#include "windowsx.h"
#include <wrl/client.h>

/*** Directx11 include files ***/
#include <d3d11.h> 
#include <dxgi1_4.h>
#include <d3d11_1.h>
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
	// Forward shaders
	ID3D11VertexShader* VS_common = nullptr;
	ID3D11PixelShader* PS_directional = nullptr;
	ID3D11PixelShader* PS_point = nullptr;
	ID3D11PixelShader* PS_spot = nullptr;
	ID3D11PixelShader* PS_emissive = nullptr;

	// Deferred shaders
	
	// Geometry pass
	ID3D11VertexShader* VS_deferred = nullptr;
	ID3D11PixelShader* PS_deferred = nullptr;

	// Light pass
	ID3D11VertexShader* VS_deferred_common = nullptr;
	ID3D11PixelShader* PS_deferred_directional = nullptr;
	ID3D11PixelShader* PS_deferred_passthrough = nullptr;

	// Deferred post process
	ID3D11PixelShader* PS_deferred_emissive = nullptr;

	// Emissive downsample
	ID3D11PixelShader* PS_deferred_emissive_downsample;

	// Skybox
	ID3D11VertexShader* VS_deferred_skybox = nullptr;
	ID3D11PixelShader* PS_deferred_skybox = nullptr;
};

enum class ShaderType {
	DirectionalLight = 0,
	SpotLight,
	PointLight,
	GeometryPass,
	DeferredDirectional,
	DeferredPointLight,
	DeferredSpotLight,
	DeferredPassThrough,
	DeferredEmissive,
	BloomDownsample,
	Skybox,

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


struct Vertex {
	float pos_x, pos_y, pos_z;
	float nrm_x, nrm_y, nrm_z;
	float uv_x, uv_y;
	float tan_x, tan_y, tan_z, tan_w;
};

struct VertexQuad {
	Vec3 position;
	Vec2 uv_x, uv_y;
};

struct CameraConstantBuffer {
	Mat4 view;
	Mat4 projection;
	Mat4 model;
	Vec3 camera_position;

	float metallic;
	Vec4 color;
	float roughness;
	Vec3 emissive;
	float emissive_intensity;
};

struct CameraDeferredConstantBuffer {
	Mat4 inv_view_proj;
	Vec3 camera_position;
	float padding;
};

struct EmissiveConstantBuffer {
	Vec2 texel_size;
	float bloom_intensity;
	int horizontal;
	int blend;
	Vec3 padding;
};



struct Color {
	float rgba[4];
};

constexpr unsigned int NUM_BUFFERING = 3;

struct WindowInfo {
	HWND window_handle;
	WNDCLASSEX window_info;
	ID3D11RenderTargetView* backbuffer;
	ID3D11RenderTargetView* emissive_buffer_view;
};

constexpr unsigned int NUM_MIPMAPS_EMISSIVE = (5 + 1);

struct DeferredResources {
	
	// Albedo
	ID3D11RenderTargetView* gbuffer_albedo_render_target_view;
	ID3D11Texture2D* gbuffer_albedo_texture;
	ID3D11ShaderResourceView* gbuffer_albedo_shader_resource_view;

	// Position
	ID3D11RenderTargetView* gbuffer_position_render_target_view;
	ID3D11Texture2D* gbuffer_position_texture;
	ID3D11ShaderResourceView* gbuffer_position_shader_resource_view;

	// Normals
	ID3D11RenderTargetView* gbuffer_normals_render_target_view;
	ID3D11Texture2D* gbuffer_normals_texture;
	ID3D11ShaderResourceView* gbuffer_normals_shader_resource_view;

	// Material (Metallic + Roughness + Ambient oclusion + Specular)
	ID3D11RenderTargetView* gbuffer_material_render_target_view;
	ID3D11Texture2D* gbuffer_material_texture;
	ID3D11ShaderResourceView* gbuffer_material_shader_resource_view;

	// Emissive (Input)
	ID3D11RenderTargetView* gbuffer_emissive_render_target_view;
	ID3D11Texture2D* gbuffer_emissive_texture;
	ID3D11ShaderResourceView* gbuffer_emissive_shader_resource_view;
	
	// Emissive (Output)
	ID3D11RenderTargetView* gbuffer_emissive_out_render_target_view;
	ID3D11Texture2D* gbuffer_emissive_out_texture;
	ID3D11ShaderResourceView* gbuffer_emissive_out_shader_resource_view;
	
	// Emissive (Output B)
	ID3D11RenderTargetView* gbuffer_emissive_out_b_render_target_view;
	ID3D11Texture2D* gbuffer_emissive_out_b_texture;
	ID3D11ShaderResourceView* gbuffer_emissive_out_b_shader_resource_view;

	// Emissive mipmaps
	ID3D11RenderTargetView* gbuffer_emissive_mipmap_render_target_view[NUM_MIPMAPS_EMISSIVE];
	ID3D11Texture2D* gbuffer_emissive_mipmap_texture[NUM_MIPMAPS_EMISSIVE];
	ID3D11ShaderResourceView* gbuffer_emissive_mipmap_shader_resource_view[NUM_MIPMAPS_EMISSIVE];

	ID3D11RenderTargetView* emissive_dowscaling_render_target_view[NUM_MIPMAPS_EMISSIVE];
	ID3D11Texture2D* emissive_dowscaling_texture[NUM_MIPMAPS_EMISSIVE];
	ID3D11ShaderResourceView* emissive_dowscaling_shader_resource_view[NUM_MIPMAPS_EMISSIVE];

	// Final postprocess
	ID3D11RenderTargetView* postprocess_render_target_view;
	ID3D11Texture2D* postprocess_texture;
	ID3D11ShaderResourceView* postprocess_resource_view;



	// Light
	ID3D11RenderTargetView* light_render_target_view;
	ID3D11Texture2D* light_texture;
	ID3D11ShaderResourceView* light_shader_resource_view;
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


using Microsoft::WRL::ComPtr;
struct EngineProps {

	// Swap chain props
	DXGI_SWAP_CHAIN_DESC1 scd;
	ComPtr<IDXGISwapChain3> swapChain;
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

