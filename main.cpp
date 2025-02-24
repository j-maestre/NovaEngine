#include "nova_engine.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	
	Engine engine;
	
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

	engine.init(win.get_window_info());


	while (true) {

		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}

	}

	engine.release();

	return 0;
}

