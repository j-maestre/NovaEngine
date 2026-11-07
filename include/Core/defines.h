#pragma once

#include <vector>

#define DIRECTX11
#define ENABLE_IMGUI
#define MEASURE_TIME
#define DEPTH_PREPASS


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

constexpr float DEFAULT_ROUGHNESS = 0.6f;

constexpr unsigned int ssao_kernel = 64;
constexpr unsigned int ssao_noise = 16;
constexpr unsigned int ssao_noise_dim = 4;


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


#include <stdio.h>
#include <string>
#include <cmath>

// Seno entre el minimo y 1. Con la misma velocidad, emisivos y luces van a la vez.
inline float pulse_factor(float time, float speed, float minimum) {
	if (speed == 0.0f) return 1.0f;
	return minimum + (1.0f - minimum) * (0.5f + 0.5f * sinf(time * speed));
}

struct ShaderFiles {
	// Forward shaders
	ID3D11VertexShader* VS_common = nullptr;
	ID3D11PixelShader* PS_directional = nullptr;
	ID3D11PixelShader* PS_point = nullptr;
	ID3D11PixelShader* PS_spot = nullptr;

	// Deferred shaders
	
	// Geometry pass
	ID3D11VertexShader* VS_deferred = nullptr;
	ID3D11PixelShader* PS_deferred = nullptr;

	// Light pass
	ID3D11VertexShader* VS_deferred_common = nullptr;
	ID3D11PixelShader* PS_deferred_composite = nullptr;
	ID3D11PixelShader* PS_deferred_fxaa = nullptr;
	ID3D11PixelShader* PS_deferred_lights = nullptr;

	// Emissive downsample
	ID3D11PixelShader* PS_deferred_emissive_downsample = nullptr;

	// Skybox
	ID3D11VertexShader* VS_deferred_skybox = nullptr;
	ID3D11PixelShader* PS_deferred_skybox = nullptr;

	// Depth Pre pass: no tiene pixel shader
	ID3D11VertexShader* VS_depth_prepass = nullptr;		// Un objeto por draw: lo usa la seleccion con clic
	ID3D11VertexShader* VS_depth_batch = nullptr;		// Toda la escena en un draw

	// Seleccion con clic
	ID3D11PixelShader* PS_entity_id = nullptr;

	// SSAO
	ID3D11PixelShader* PS_SSAO = nullptr;
	ID3D11VertexShader* VS_SSAO = nullptr;
	ID3D11PixelShader* PS_SSAO_Blur_blend = nullptr;
};

enum class ShaderType {
	DirectionalLight = 0,
	SpotLight,
	PointLight,
	GeometryPass,
	DeferredLights,
	DeferredComposite,
	Fxaa,
	BloomDownsample,
	Skybox,
	Depth,
	EntityId,
	SSAO,
	BlurSSAO,
};

enum class DrawMode {
	Solid = 0,
	Wireframe,
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
	Mat4 normal_matrix;
	Vec2 uv_scale;
	float emissive_bloom;
	float padding_object;
};

struct CameraDeferredConstantBuffer {
	Mat4 inv_view_proj;
	Vec3 camera_position;
	float cubemap_max_mip_level;
};

struct CameraDepthPrePass {
	Mat4 view;
	Mat4 projection;
	Mat4 model;
};

struct EmissiveConstantBuffer {
	Vec2 texel_size;
	float bloom_intensity;
	int horizontal;
	int blend;
	Vec3 padding;
};



struct SSAOConstantBuffer {
	Mat4 projection;
	Mat4 view;
	Mat4 inv_projection;

	Vec4 kernel_samples[ssao_kernel];
	int samples;
	float ssao_base_radius;
	float samples_float;
	float width;
	float height;
	float max_radius_px;
	float resolution_scale;		// 1 = resolucion completa, 2 = media
	float padding_ssao;
};


enum class GPULightType : int {
	Directional = 0,
	Point = 1,
	Spot = 2,
};

// Debe coincidir con GPULight en ps_deferred_lights.hlsl
struct GPULight {
	Vec3 position;
	GPULightType type;
	Vec3 direction;
	float intensity;
	Vec3 color;
	float range;
	float cos_inner;
	float cos_outer;
	float fall_start;
	float constant_att;
	float linear_att;
	float quadratic_att;
	Vec2 padding;
};
static_assert(sizeof(GPULight) == 80, "GPULight debe coincidir con el shader");

struct LightPassConstantBuffer {
	uint32 light_count;
	float ambient_intensity;
	float ssao_power;
	float ssao_direct;
};

enum class Tonemapper : int {
	Exponential = 0,
	ACES = 1,
};

struct EntityIdConstantBuffer {
	uint32 entity_id;
	Vec3 padding;
};

struct CompositeConstantBuffer {
	float exposure;
	Tonemapper tonemapper;
	Vec2 padding;
};

struct FxaaConstantBuffer {
	Vec2 texel_size;
	int enabled;
	float padding;
};

struct Color {
	float rgba[4];
};

constexpr unsigned int NUM_BUFFERING = 3;

struct WindowInfo {
	HWND window_handle;
	WNDCLASSEX window_info;
	ID3D11RenderTargetView* backbuffer;
};

constexpr unsigned int NUM_MIPMAPS_EMISSIVE = (5 + 1);

struct DeferredResources {
	
	// Albedo
	ID3D11RenderTargetView* gbuffer_albedo_render_target_view;
	ID3D11Texture2D* gbuffer_albedo_texture;
	ID3D11ShaderResourceView* gbuffer_albedo_shader_resource_view;

	// Position

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

	// Imagen final ya en sRGB, antes del FXAA
	ID3D11RenderTargetView* fxaa_input_render_target_view;
	ID3D11Texture2D* fxaa_input_texture;
	ID3D11ShaderResourceView* fxaa_input_shader_resource_view;

	// SSAO
	ID3D11RenderTargetView* ssao_render_target_view;
	ID3D11Texture2D* ssao_texture;
	ID3D11ShaderResourceView* ssao_shader_resource_view;

	// SSAO blur
	ID3D11RenderTargetView* ssao_blur_render_target_view;
	ID3D11Texture2D* ssao_blur_texture;
	ID3D11ShaderResourceView* ssao_blur_shader_resource_view;

	// Vista de lectura del depth buffer. La crea y la libera Window, junto con el depth buffer.
	ID3D11ShaderResourceView* depth_srv;

	// IDs de entidad: solo se escribe en el frame de un clic
	ID3D11RenderTargetView* entity_id_rtv;
	ID3D11Texture2D* entity_id_texture;
	ID3D11ShaderResourceView* entity_id_srv;
};



struct RenderInfo {
	int ssao_samples = 16;
	float ssao_base_radius = 1.5f;
	float ssao_max_radius_px = 128.0f;
	float ssao_intensity = 2.0f;
	float ssao_direct = 0.5f;
	float exposure = 1.0f;
	Tonemapper tonemapper = Tonemapper::ACES;
	float ambient_intensity = 0.1f;
	bool ssao_active = true;
	bool ssao_half_res = true;
	bool bloom_active = true;
	bool fxaa_active = true;
	bool frustum_culling = true;
	DrawMode draw_mode = DrawMode::Solid;
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

