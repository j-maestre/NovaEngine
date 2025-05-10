
#include "render/window.h"
#include "Core/defines.h"
#include "Core/input.h"
#include "Windows.h"
#include "render/imgui/imgui_manager.h" 
#include "imgui.h"
#include "imgui/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {

	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, GWLP_USERDATA));

	/*
	*/

	if (ImGui_ImplWin32_WndProcHandler(handle, msg, wParam, lParam)) {
		return true;
	}

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
}

Window::Window(Window&& other) : m_window_info(other.m_window_info){
	m_initialized = other.m_initialized;
	other.m_window_info.reset();
	other.m_initialized = false;
}

Window::~Window(){

}

void Window::init(const WindowProperties* props){

	

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

	RECT wr = { 0.0f, 0.0f, props->width, props->height};    // set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size


	// Create window and use the result as the handle
	window_handle = CreateWindowEx(NULL, props->name.c_str(), props->name.c_str(), WS_OVERLAPPEDWINDOW,
		props->pos_x,		// x position 
		props->pos_y,		// y position
		props->width,		// width
		props->height,		// height
		NULL,				// parent window
		NULL,				// Menus
		props->hInstance,	// aplication handle
		NULL				// Used with multiple windows
	);

	m_width = static_cast<float>(props->width);
	m_height = static_cast<float>(props->height);

	// Store this class in the window class
	SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	WindowInfo tmp;
	tmp.window_handle = window_handle;
	tmp.window_info = window_info;

	m_window_info = std::make_shared<WindowInfo>(tmp);
	m_initialized = true;

	m_window_props = std::make_shared<WindowProperties>(*props);


	// Display window on screen
	ShowWindow(window_handle, props->nCmdShow);

}

void Window::init_imgui(){

	m_imgui->init(m_window_info->window_handle);
}

void Window::begin_frame(){
	
	if (m_input->is_key_down(Key::Keyboard::ESCAPE)) {
		PostQuitMessage(0);
	}

	m_inmediateDeviceContext->ClearRenderTargetView(m_window_info->backbuffer, (FLOAT*) &(m_window_props->clear_color));
	for (auto& key : m_input->m_keyboard) {

		switch (key.second) {
			case Key::KeyState::Down: key.second = Key::KeyState::Pressed; break;
			case Key::KeyState::Release: key.second = Key::KeyState::None; break;
			default:break;
		}
		
	}

	m_imgui->begin_frame();
}

void Window::end_frame(){

	

	// Switch the backbuffer and the front buffer
	m_imgui->end_frame();
	m_swapChain->Present(0,0);
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
	
	m_imgui->show_demo_window();
	return ret;
}

void Window::release(){
	m_window_info->backbuffer->Release();
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
	GetCursorPos(&cursorPos);  // Obtener las coordenadas globales del ratón

	ScreenToClient(handle, &cursorPos);  // Convertir a coordenadas relativas a la ventana

	RECT windowRect;
	GetClientRect(handle, &windowRect);  // Obtener el tamaño de la ventana

	// Normalizar las coordenadas del ratón a un rango entre 0 y 1
	m_input->m_mouse_x = static_cast<float>(cursorPos.x) / static_cast<float>(windowRect.right - windowRect.left);
	m_input->m_mouse_y = static_cast<float>(cursorPos.y) / static_cast<float>(windowRect.bottom - windowRect.top);
	*/


}

void Window::process_mouse_button(Key::Mouse btn, Key::KeyState state){
	m_input->m_mouse[btn] = state;
}


