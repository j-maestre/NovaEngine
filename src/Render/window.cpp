
#include "render/window.h"
#include "Core/defines.h"


LRESULT CALLBACK WindowProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_DESTROY:
	{
		// close the entire aplication
		PostQuitMessage(0);
		return 0;
	}
	break;
	}

	// Handle other messages
	return DefWindowProc(handle, msg, wParam, lParam);
}

Window::Window() : m_window_info(){
	m_initialized = false;
	m_swapChain = nullptr;
	m_deviceInterface = nullptr;
	m_inmediateDeviceContext = nullptr;
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
	window_info.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_info.lpszClassName = props->name.c_str();

	// Register window class
	RegisterClassEx(&window_info);

	RECT wr = { 0, 0, props->width, props->height};    // set the size, but not the position
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

	WindowInfo tmp;
	tmp.window_handle = window_handle;
	tmp.window_info = window_info;

	m_window_info = std::make_shared<WindowInfo>(tmp);
	m_initialized = true;

	m_window_props = std::make_shared<WindowProperties>(*props);
	// Display window on screen
	ShowWindow(window_handle, props->nCmdShow);

}

void Window::begin_frame(){
	
	m_inmediateDeviceContext->ClearRenderTargetView(m_window_info->backbuffer, (FLOAT*) &(m_window_props->clear_color));
}

void Window::end_frame(){

	// Switch the backbuffer and the front buffer
	m_swapChain->Present(0,0);
}

bool Window::update(){

	// Window event messages

	bool ret = true;
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		// Translate keystroke messages into the right format
		TranslateMessage(&msg);
		// Send message to the WinProc function
		DispatchMessage(&msg);

		ret = msg.message != WM_QUIT;
	}
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


