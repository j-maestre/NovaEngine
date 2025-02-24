#include "Core/engine.h"

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



Engine::Engine() : m_props(std::make_shared<EngineProps>()){
	
}

/*
Engine::Engine(const Engine&){

}
*/

Engine::~Engine(){

}

void Engine::init(const WindowInfo* window_used){
	
	if (window_used != nullptr) {
		//window_used->window_handle


		// clear out the struct for use
		ZeroMemory(&(m_props->scd), sizeof(DXGI_SWAP_CHAIN_DESC));
		
		m_props->scd.BufferCount = 1;	// One back buffer
		m_props->scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 32 bit color
		m_props->scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// How swap chain is going to be used
		m_props->scd.OutputWindow = window_used->window_handle;			// The window to be used
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
	}
}

void Engine::release(){
	m_props->swapChain->Release();
	m_props->deviceInterface->Release();
	m_props->inmediateDeviceContext->Release();

}

void Engine::update(){

	
}
