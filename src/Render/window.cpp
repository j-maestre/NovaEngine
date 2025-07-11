
#include "render/window.h"
#include "Core/defines.h"
#include "Core/input.h"
#include "Windows.h"
#include "render/imgui/imgui_manager.h" 

//#include "imgui.h"
//#include "imgui/backends/imgui_impl_win32.h"
#include "Core/engine.h"
#include "render/renderer.h"


#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {

	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));

#ifdef ENABLE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(handle, msg, wParam, lParam)) {
		return true;
	}
#endif

	switch (msg) {
	case WM_DESTROY:
		{
			// close the entire aplication
			PostQuitMessage(0);
			return 0;
		}
		break;

	
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			window->process_key(wParam, true);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		window->process_key(wParam, false);
		break;
	case WM_MOUSEMOVE:
		{
		window->process_mouse(lParam);
		}
		break;
	case WM_LBUTTONDOWN: 
		window->process_mouse_button(Key::Mouse::LBUTTON, Key::KeyState::Down);
		break;

	case WM_LBUTTONUP:
		window->process_mouse_button(Key::Mouse::LBUTTON, Key::KeyState::Release);
		return 0;

	case WM_RBUTTONDOWN:
		window->process_mouse_button(Key::Mouse::RBUTTON, Key::KeyState::Down);
		return 0;

	case WM_RBUTTONUP:
		window->process_mouse_button(Key::Mouse::RBUTTON, Key::KeyState::Release);
		return 0;


	case WM_MOUSEWHEEL:
	{
		short delta_tmp = GET_WHEEL_DELTA_WPARAM(wParam); // positivo = arriba, negativo = abajo
		window->process_mouse_wheel(delta_tmp > 0);
		return 0;
	}
		

	case WM_SIZE:
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		window->resize();
		
	return 0;
	}

	// Handle other messages
	return DefWindowProc(handle, msg, wParam, lParam);
}

Window::Window() : m_window_info(){
	m_initialized = false;
	m_swapChain = nullptr;
	m_deviceInterface = nullptr;
	m_inmediateDeviceContext = nullptr;
	m_imgui = ImguiManager::get_instance();
	//m_window_info->emissive_buffer_view = nullptr;
	//m_window_info->backbuffer = nullptr;
}

Window::Window(Window&& other) : m_window_info(other.m_window_info){
	m_initialized = other.m_initialized;
	other.m_window_info.reset();
	other.m_initialized = false;
}

Window::~Window(){

}

void Window::init(WindowProperties* props){

	

	HWND window_handle;
	WNDCLASSEX window_info;

	// Clear out the window class for use
	ZeroMemory(&window_info, sizeof(WNDCLASSEX));

	window_info.cbSize = sizeof(WNDCLASSEX);
	window_info.style = CS_HREDRAW | CS_VREDRAW;
	window_info.lpfnWndProc = WindowProc;
	window_info.hInstance = props->hInstance;
	window_info.hCursor = LoadCursor(NULL, IDC_ARROW);
	//window_info.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_info.lpszClassName = props->name.c_str();

	// Register window class
	RegisterClassEx(&window_info);

	RECT wr = { 
		(LONG)0.0,
		(LONG)0.0,
		(LONG)props->width,
		(LONG)props->height};    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size
	int window_width = wr.right - wr.left;
	int window_height = wr.bottom - wr.top;

	window_width &= ~1;
	window_height &= ~1;


	// Create window and use the result as the handle
	window_handle = CreateWindowEx(NULL, props->name.c_str(), props->name.c_str(), WS_OVERLAPPEDWINDOW,
		props->pos_x,		// x position 
		props->pos_y,		// y position
		window_width,		// width
		window_height,		// height
		NULL,				// parent window
		NULL,				// Menus
		props->hInstance,	// aplication handle
		NULL				// Used with multiple windows
	);

	// TEST
	RECT clientRect;
	GetClientRect(window_handle, &clientRect);
	printf("Client area: %d x %d\n", clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	//

	m_width = static_cast<float>(window_width);
	m_height = static_cast<float>(window_height);

	props->width = window_width;
	props->height = window_height;


	WindowInfo tmp;
	tmp.window_handle = window_handle;
	tmp.window_info = window_info;

	m_window_info = std::make_shared<WindowInfo>(tmp);
	m_initialized = true;

	m_window_props = std::make_shared<WindowProperties>(*props);

	m_window_info->emissive_buffer_view = nullptr;
	//for (int i = 0; i < NUM_BUFFERING; i++) {
		m_window_info->backbuffer = nullptr;
	//}
	
	// Store this class in the window class
	SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Display window on screen
	ShowWindow(window_handle, props->nCmdShow);

	
}

void Window::init_imgui(){

#ifdef ENABLE_IMGUI
	m_imgui->init(m_window_info->window_handle);
	resize();
#endif
}

void Window::begin_frame(){
	
#ifdef MEASURE_TIME
	auto start = std::chrono::high_resolution_clock::now();
#endif

	if (m_input->is_key_down(Key::Keyboard::ESCAPE)) {
		PostQuitMessage(0);
	}

	m_inmediateDeviceContext->ClearRenderTargetView(m_window_info->backbuffer, (FLOAT*)&(m_window_props->clear_color));
	for (auto& key : m_input->m_keyboard) {

		switch (key.second) {
			case Key::KeyState::Down: key.second = Key::KeyState::Pressed; break;
			case Key::KeyState::Release: key.second = Key::KeyState::None; break;
			default:break;
		}
		
	}
	
	for (auto& key : m_input->m_mouse) {

		switch (key.second) {
			case Key::KeyState::Down: key.second = Key::KeyState::Pressed; break;
			case Key::KeyState::Release: key.second = Key::KeyState::None; break;
			default:break;
		}
		
	}

	m_input->m_mouse_wheel_up = false;
	m_input->m_mouse_wheel_down = false;

#ifdef ENABLE_IMGUI
	m_imgui->begin_frame();
#endif


#ifdef MEASURE_TIME
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	m_imgui->m_window_begin_frame_time = std::chrono::duration<float, std::milli>(elapsed).count();
#endif
}

void Window::end_frame(){

#ifdef MEASURE_TIME
	auto start = std::chrono::high_resolution_clock::now();
#endif

#ifdef ENABLE_IMGUI
	m_imgui->end_frame();
#endif

	
	
	// Switch the backbuffer and the front buffer
	m_swapChain->Present(m_vsync,0);

	//UINT currentIndex = m_swapChain->GetCurrentBackBufferIndex();
	//printf("Current backbuffer index %u\n", currentIndex);
	
#ifdef MEASURE_TIME
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	m_imgui->m_window_end_frame_time = std::chrono::duration<float, std::milli>(elapsed).count();
#endif

}

bool Window::update(){

	// Window event messages

	bool ret = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && ret) {
		// Translate keystroke messages into the right format
		TranslateMessage(&msg);
		// Send message to the WinProc function
		DispatchMessage(&msg);

		ret = msg.message != WM_QUIT;
	}
	
	
	return ret;
}

void Window::release(){

	//for (int i = 0; i < NUM_BUFFERING; i++) {
		m_window_info->backbuffer->Release();
	//}
}

WindowInfo* Window::get_window_info(){

	return m_window_info.get();
}

WindowProperties* Window::get_window_properties(){

	return m_window_props.get();
}

void Window::process_key(WPARAM param, bool down){

	Key::Keyboard key = static_cast<Key::Keyboard>(param);
	if (down) {
		if (m_input->m_keyboard[key] != Key::KeyState::Pressed) {
			m_input->m_keyboard[key] = Key::KeyState::Down;
		}
	}else {
		m_input->m_keyboard[key] = Key::KeyState::Release;
	}

}

void Window::process_mouse(LPARAM param){
	
	m_input->m_mouse_x = GET_X_LPARAM(param);
	m_input->m_mouse_y = GET_Y_LPARAM(param);

	/*
	POINT cursorPos;
	GetCursorPos(&cursorPos);  // Obtener las coordenadas globales del rat�n

	ScreenToClient(handle, &cursorPos);  // Convertir a coordenadas relativas a la ventana

	RECT windowRect;
	GetClientRect(handle, &windowRect);  // Obtener el tama�o de la ventana

	// Normalizar las coordenadas del rat�n a un rango entre 0 y 1
	m_input->m_mouse_x = static_cast<float>(cursorPos.x) / static_cast<float>(windowRect.right - windowRect.left);
	m_input->m_mouse_y = static_cast<float>(cursorPos.y) / static_cast<float>(windowRect.bottom - windowRect.top);
	*/


}

void Window::process_mouse_button(Key::Mouse btn, Key::KeyState state){
	m_input->m_mouse[btn] = state;
}

void Window::process_mouse_wheel(bool up){

	if (up) m_input->m_mouse_wheel_up = true;
	else m_input->m_mouse_wheel_down = true;

}

void Window::set_full_screen(){

	IDXGIDevice* device;
	Engine::get_instance()->get_engine_props()->swapChain->GetDevice(IID_PPV_ARGS(&device));

	IDXGIAdapter* adapter = nullptr;
	HRESULT hr = device->GetAdapter(&adapter);

	if (SUCCEEDED(hr)) {
		IDXGIOutput* output = nullptr;
		hr = adapter->EnumOutputs(0, &output);

		if (SUCCEEDED(hr)) {
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			// Get resolution or other parameters...
		
			Engine::get_instance()->get_engine_props()->swapChain->SetFullscreenState(TRUE, output);
		
		}
	}
}

void Window::set_windowed()
{
}

void Window::resize(){
	Engine* e = Engine::get_instance();

	if (e->get_engine_props()->inmediateDeviceContext) {

		destroy_frame_resources();
		m_renderer->release_deferred_resources();
		
		create_frame_resources();
	}

}


void Window::destroy_frame_resources() {

	

	Engine::get_instance()->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(0, 0, 0);

	//for (int i = 0; i < NUM_BUFFERING; i++) {
		if (m_renderer->m_backbuffer_texture)m_renderer->m_backbuffer_texture->Release();
	//}
	if (m_renderer->m_depth_buffer)m_renderer->m_depth_buffer->Release();
	if (m_renderer->m_depth_stencil_view)m_renderer->m_depth_stencil_view->Release();

	
	// Release render target view
	//for (int i = 0; i < NUM_BUFFERING; i++) {
		if (m_window_info->backbuffer)m_window_info->backbuffer->Release();
		m_window_info->backbuffer = nullptr;
	//}

	// Release emissive buffer
	if (m_renderer->m_emissive_texture) {
		m_renderer->m_emissive_texture->Release();
		m_renderer->m_emissive_texture = nullptr;
	}

	if (m_renderer->m_emissive_SRV) {
		m_renderer->m_emissive_SRV->Release();
		m_renderer->m_emissive_SRV = nullptr;
	}

	if (m_window_info->emissive_buffer_view) {
		m_window_info->emissive_buffer_view->Release();
		m_window_info->emissive_buffer_view = nullptr;
	}

}

void Window::create_frame_resources(){

	Engine* e = Engine::get_instance();

	// Make sure its pair
	RECT clientRect;
	GetClientRect(m_window_info->window_handle, &clientRect);
	UINT width_w = clientRect.right - clientRect.left;
	UINT height_w = clientRect.bottom - clientRect.top;

	width_w &= ~1;
	height_w &= ~1;

	D3D11_TEXTURE2D_DESC depth_desc{
		.Width = width_w,
		.Height = height_w,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc{
			.Count = 1
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL
	};

	m_renderer->m_depth_buffer = nullptr;
	m_renderer->m_depth_stencil_view = nullptr;
	e->get_engine_props()->deviceInterface->CreateTexture2D(&depth_desc, nullptr, &(m_renderer->m_depth_buffer));
	e->get_engine_props()->deviceInterface->CreateDepthStencilView(m_renderer->m_depth_buffer, nullptr, &(m_renderer->m_depth_stencil_view));

	e->get_engine_props()->swapChain->ResizeBuffers(NUM_BUFFERING, width_w, height_w, DXGI_FORMAT_UNKNOWN, 0);


	m_window_props->width = width_w;
	m_window_props->height = height_w;

	// Backbuffer

	ID3D11Texture2D* backBuffer = nullptr;
	e->get_engine_props()->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

	D3D11_TEXTURE2D_DESC desc;
	backBuffer->GetDesc(&desc);
	printf("Backbuffer size: %u x %u\n", desc.Width, desc.Height);

	e->get_engine_props()->deviceInterface->CreateRenderTargetView(backBuffer, nullptr, &m_window_info->backbuffer);
	m_renderer->m_backbuffer_texture = backBuffer;
	

	e->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window_info->backbuffer, nullptr);

	D3D11_VIEWPORT viewport{};
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (FLOAT)width_w;
	viewport.Height = (FLOAT)height_w;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	e->m_viewport = viewport;
	e->get_engine_props()->inmediateDeviceContext->RSSetViewports(1, &e->m_viewport);


	
	// Emissive buffer texture creation
	ID3D11Texture2D* emissive_buffer = nullptr;
	D3D11_TEXTURE2D_DESC emissive_desc{
		.Width = width_w,
		.Height = height_w,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR compatible format
		.SampleDesc{
			.Count = 1,
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
	};

	HRESULT hr = e->get_engine_props()->deviceInterface->CreateTexture2D(&emissive_desc, nullptr, &emissive_buffer);
	if (FAILED(hr)) {
		printf("\n*** Error creating emissive buffer ***\n");
		assert("Error creating emissive buffer");
	}
	m_renderer->m_emissive_texture = emissive_buffer;
	
	// Emissive buffer render target view
	hr = e->get_engine_props()->deviceInterface->CreateRenderTargetView(m_renderer->m_emissive_texture, nullptr, &m_window_info->emissive_buffer_view);
	if (FAILED(hr)) {
		printf("\n*** Error creating emissive render target view ***\n");
		assert("Error creating emissive render target view");
	}

	// Emissive buffer shader resource view
	hr = e->get_engine_props()->deviceInterface->CreateShaderResourceView(m_renderer->m_emissive_texture, nullptr, &m_renderer->m_emissive_SRV);
	if (FAILED(hr)) {
		printf("\n*** Error creating emissive shader resource view ***\n");
		assert("Error creating emissive shader resource view");
	}

	create_full_screen_quad_resources(width_w, height_w);

	m_renderer->create_deferred_resources(width_w, height_w);
#ifdef ENABLE_IMGUI
	ImguiManager::get_instance()->resize(m_window_info->window_handle, width_w, height_w);
#endif

}

void Window::create_full_screen_quad_resources(unsigned int width, unsigned int height){

	Engine* e = Engine::get_instance();

	ID3D11Texture2D* scene_render_target_texture = nullptr;
	D3D11_TEXTURE2D_DESC rt_desc{};
	rt_desc.Width = width;
	rt_desc.Height = height;
	rt_desc.MipLevels = 1;
	rt_desc.ArraySize = 1;
	rt_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;	// HDR format
	rt_desc.SampleDesc.Count = 1;
	rt_desc.SampleDesc.Quality = 0;
	rt_desc.Usage = D3D11_USAGE_DEFAULT;
	rt_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rt_desc.CPUAccessFlags = 0;
	rt_desc.MiscFlags = 0;

	HRESULT hr = e->get_engine_props()->deviceInterface->CreateTexture2D(&rt_desc, nullptr, &scene_render_target_texture);
	if (FAILED(hr)) {
		printf("*** Error creating scene render target texture ***\n");
		assert(false);
	}

	ID3D11RenderTargetView* scene_render_target_view = nullptr;
	hr = e->get_engine_props()->deviceInterface->CreateRenderTargetView(scene_render_target_texture, nullptr, &scene_render_target_view);
	if (FAILED(hr)) {
		printf("*** Error creating scene render target view ***\n");
		assert(false);
	}

	ID3D11ShaderResourceView* scene_SRV = nullptr;
	hr = e->get_engine_props()->deviceInterface->CreateShaderResourceView(scene_render_target_texture, nullptr, &scene_SRV);
	if (FAILED(hr)) {
		printf("*** Error creating scene shader resource view ***\n");
		assert(false);
	}

	m_renderer->m_quad_texture = scene_render_target_texture;
	m_renderer->m_quad_RTV = scene_render_target_view;
	m_renderer->m_quad_SRV = scene_SRV;
}


