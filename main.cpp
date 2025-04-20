#include "nova_engine.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	
	Engine engine;
	
	Window win;
	WindowProperties props;
	Color c = { 1.0f, 0.1f, 0.5f, 1.0f };
	props.clear_color = c;
	props.width = SCREEN_WIDTH;
	props.height = SCREEN_HEIGHT;
	props.hInstance = hInstance;
	props.name = L"Window de locos";
	props.nCmdShow = nCmdShow;
	props.pos_x = 0;
	props.pos_y = 0;

	win.init(&props);
	engine.init(&win);

	Renderer render;
	bool ret = render.init_pipeline(&engine);


	while (true) {

		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}

		engine.update();
		win.begin_frame();
		
		render.draw_triangle();
		
		win.end_frame();

	}

	engine.release();

	return 0;
}

