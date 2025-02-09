
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

	while (win.update()) {

	}

	return 0;
}

