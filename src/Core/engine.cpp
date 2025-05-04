#include "Core/engine.h"
#include <Core/defines.h>
#include <Core/input.h>
#include <assert.h>
#include <chrono>
#include <Windows.h>

Engine::Engine() : m_props(std::make_shared<EngineProps>()), m_input(), m_resource(){

	m_resource.set_engine(this);
	m_last_time = std::chrono::high_resolution_clock::now();
	
}

/*
Engine::Engine(const Engine&){

}
*/

Engine::~Engine(){
	m_raster_state->Release();
}

void Engine::init(Window* window){
	
	assert(window != nullptr);

	//window_used->window_handle

	window->m_input = &m_input;

	// clear out the struct for use
	ZeroMemory(&(m_props->scd), sizeof(DXGI_SWAP_CHAIN_DESC));

	WindowInfo* window_info = window->get_window_info();
		
	m_props->scd.BufferCount = 1;	// One back buffer
	m_props->scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 32 bit color
	m_props->scd.BufferDesc.Width = SCREEN_WIDTH;					// Backbuffer width
	m_props->scd.BufferDesc.Height = SCREEN_HEIGHT;					// Backbuffer height
	m_props->scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// How swap chain is going to be used
	m_props->scd.OutputWindow = window_info->window_handle;			// The window to be used
	m_props->scd.SampleDesc.Count = 1;								// How many multisamples (anti aliasing, TODO: maybe change to 1 or parametrize in the future) (minium 1 max 4)
	m_props->scd.Windowed = TRUE;
	m_props->scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// Allows full screen switching
	//m_props->driverType = D3D_DRIVER_TYPE_HARDWARE;		// Uses advance gpu hardware for rendering	TODO: enable to check performance
	// m_props->driverType = D3D_DRIVER_TYPE_REFERENCE;		// If your hardware can't run D3D11
	// m_props->driverType = D3D_DRIVER_TYPE_NULL;			// for non-graphics purpouses
#ifdef _DEBUG
	m_props->flags |= D3D11_CREATE_DEVICE_DEBUG;				// Debug
#endif
	//m_props->flags |= D3D11_CREATE_DEVICE_SWITCH_TO_REF;		// Change hardware mode to reference mode
	//m_props->flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;		// Allows Direct2D working with Direct3D
	//m_props->flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;		// Singlethread



	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		m_props->flags,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&(m_props->scd),
		&(m_props->swapChain),
		&(m_props->deviceInterface),
		NULL,
		&(m_props->inmediateDeviceContext));


	// Setting rasterizer
	ZeroMemory(&m_raster, sizeof(D3D11_RASTERIZER_DESC));
	m_raster.CullMode = D3D11_CULL_BACK;    // Desactiva el culling
	m_raster.FillMode = D3D11_FILL_SOLID;   // Rellenar las caras con un color solido
	m_raster.FrontCounterClockwise = FALSE; // La orientacion de las caras frontales no cambia
	m_raster.DepthBias = 0;
	m_raster.SlopeScaledDepthBias = 0.0f;
	m_raster.DepthBiasClamp = 0.0f;
	m_raster.ScissorEnable = FALSE;         // No se usa el scissor test
	m_raster.MultisampleEnable = FALSE;     // No multisampling
	m_raster.AntialiasedLineEnable = FALSE; // No se usan lineas antialiasing

	HRESULT hr = m_props->deviceInterface->CreateRasterizerState(&m_raster, &m_raster_state);
	assert(!FAILED(hr));
	m_props->inmediateDeviceContext->RSSetState(m_raster_state);
	
	



	// Setting the viewport
	WindowProperties* win_props = window->get_window_properties();
	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftX = 0;
	m_viewport.Width = win_props->width; // SCREEN_WIDTH
	m_viewport.Height = win_props->height; // SCREEN_HEIGHT
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_props->inmediateDeviceContext->RSSetViewports(1, &m_viewport);
	window->m_swapChain = m_props->swapChain;
	window->m_deviceInterface = m_props->deviceInterface;
	window->m_inmediateDeviceContext = m_props->inmediateDeviceContext;
	
	load_default_textures();
}


void Engine::update(){

	m_props->inmediateDeviceContext->RSSetViewports(1, &m_viewport);

	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = currentTime - m_last_time;
	m_last_time = currentTime;
	m_delta_time = delta.count();
}

void Engine::release(){
	m_props->swapChain->SetFullscreenState(FALSE, NULL);	// TODO: method to change full screen?
	m_props->swapChain->Release();
	m_props->deviceInterface->Release();
	m_props->inmediateDeviceContext->Release();

}

void Engine::open_console(){
	// Open console
	AllocConsole(); // Crea una nueva consola
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout); // Redirige stdout a la consola
	freopen_s(&file, "CONOUT$", "w", stderr); // Redirige stderr a la consola
	freopen_s(&file, "CONIN$", "r", stdin);   // Redirige stdin a la consola
	printf("\n*** Console opened succesfully ***\n");
}

float Engine::get_delta_time(){

	return m_delta_time;
}

float Engine::get_fps(){

	return 1.0f / m_delta_time;
}

void Engine::load_default_textures(){
	Texture* t = m_resource.load_texture("data/textures/jou.jpg");
	Texture* t2 = m_resource.load_texture("data/textures/kirby.png");

	m_default_texture_albedo = t->get_id();
}

Texture* Engine::get_default_albedo_texture(){

	return m_resource.get_texture(m_default_texture_albedo);
}

void Engine::init_geometries(){


	m_cube_model =  m_resource.load_mesh("data/models/basics/cube.fbx");
	// TODO: make sphere
	
}
