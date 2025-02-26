#include "Core/engine.h"
#include <Core/defines.h>
#include <assert.h>




Engine::Engine() : m_props(std::make_shared<EngineProps>()){
	
}

/*
Engine::Engine(const Engine&){

}
*/

Engine::~Engine(){

}

void Engine::init(Window* window){
	
	assert(window != nullptr);

	//window_used->window_handle


	// clear out the struct for use
	ZeroMemory(&(m_props->scd), sizeof(DXGI_SWAP_CHAIN_DESC));

	WindowInfo* window_info = window->get_window_info();
		
	m_props->scd.BufferCount = 1;	// One back buffer
	m_props->scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 32 bit color
	m_props->scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// How swap chain is going to be used
	m_props->scd.OutputWindow = window_info->window_handle;			// The window to be used
	m_props->scd.SampleDesc.Count = 4;								// How many multisamples (anti aliasing, TODO: maybe change to 1 or parametrize in the future) (minium 1 max 4)
	m_props->scd.Windowed = TRUE;
	m_props->driverType = D3D_DRIVER_TYPE_HARDWARE;			// Uses advance gpu hardware for rendering		
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

	// Setting the back buffer
	ID3D11Texture2D* pBackbuffer;
	m_props->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackbuffer);					// __uuidof gets the unique id of the COM object
	m_props->deviceInterface->CreateRenderTargetView(pBackbuffer, NULL, &(window_info->backbuffer));
	m_props->inmediateDeviceContext->OMSetRenderTargets(1, &(window_info->backbuffer), NULL);			// last argument is depth stencill view



	// Setting the viewport
	WindowProperties* win_props = window->get_window_properties();
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftX = 0;
	viewport.Width = win_props->width;
	viewport.Height = win_props->height;

	m_props->inmediateDeviceContext->RSSetViewports(1, &viewport);
	window->m_swapChain = m_props->swapChain;
	window->m_deviceInterface = m_props->deviceInterface;
	window->m_inmediateDeviceContext = m_props->inmediateDeviceContext;
	
}


void Engine::update(){

	
}

void Engine::release(){
	m_props->swapChain->Release();
	m_props->deviceInterface->Release();
	m_props->inmediateDeviceContext->Release();

}

float Engine::get_delta_time(){

	return 0.0f;
}

float Engine::get_fps(){

	return 0.0f;
}
