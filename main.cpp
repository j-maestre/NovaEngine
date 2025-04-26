#include "nova_engine.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	
	Engine engine;
	
	Window win;
	WindowProperties props;
	Color c = { 0.1f, 0.1f, 0.1f, 1.0f };
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

	CameraComponent cam;

	float scale = 1.0f;

	TransformComponent trans({-2.0f, -4.0f, 10.0f}, { scale ,scale ,scale }, {0.0f, 90.0f, 0.0f});
	TransformComponent trans2({0.0f, 0.0f, -40.0f}, { scale ,scale ,scale });

	render.set_camera(&cam);
	
	while (true) {

		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}

		engine.update();
		win.begin_frame();

		/*
		Vec3 pos = cam.get_position();
		//pos.y += 0.9f * engine.get_delta_time();
		cam.set_position(pos);
		*/

		Vec3 rotation = trans.get_rotation();
		trans.rotateY(rotation.y + 0.2f * engine.get_delta_time());

		cam.update();
		trans.update();
		trans2.update();

		
		render.render_forward(&trans);
		//render.render_forward(&trans2);
		
		win.end_frame();
		trans.force_update();
		trans2.force_update();

	}

	engine.release();

	return 0;
}

