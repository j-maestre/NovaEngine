
#include "render/window.h"
#include "Core/defines.h"


Window::Window() : m_window_info(){
	m_initialized = false;
}

Window::Window(const Window&){

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
	window_info.hbrBackground = props->clear_color;
	window_info.lpszClassName = props->name.c_str();

	// Register window class
	RegisterClassEx(&window_info);

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
	
	// Display window on screen
	ShowWindow(window_handle, props->nCmdShow);

}
