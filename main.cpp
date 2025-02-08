
#include "Core/defines.h"
#include "render/window.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	Window win;
	struct WindowProperties props;
	props.clear_color = (HBRUSH)COLOR_WINDOW;
	props.width = 1200;
	props.height = 900;
	props.hInstance = hInstance;
	props.name = L"Window de locos";
	props.nCmdShow = nCmdShow;
	props.pos_x = 0;
	props.pos_y = 0;
	
	win.init(&props);

	// Window event messages
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {

		// Translate keystroke messages into the right format
		TranslateMessage(&msg);

		// Send message to the WinProc function
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

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
