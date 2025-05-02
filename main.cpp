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

	CameraComponent cam(engine.get_input());

	float scale = 1.0f;

	TransformComponent trans({-2.0f, -4.0f, 10.0f}, { scale ,scale ,scale }, {0.0f, 90.0f, 0.0f});
	TransformComponent trans2({0.0f, 0.0f, -10.0f}, { scale ,scale ,scale });

	render.set_camera(&cam);
	
	while (true) {

		win.begin_frame();
		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}
		//printf("hola");
		std::string dt_s = std::string("DT: ") + std::to_string(engine.get_delta_time());
		std::string fps_s = std::string("FPS: ") + std::to_string(engine.get_fps());
		//OutputDebugStringA(dt_s.c_str());
		//OutputDebugStringA(fps_s.c_str());
		//printf("%s\n", fps_s.c_str());

		
		if (engine.get_input()->is_key_down(Key::Keyboard::W)) {
			printf("Down\n");
		}
		if (engine.get_input()->is_key_pressed(Key::Keyboard::W)) {
			printf("Pressed\n");
		}
		if (engine.get_input()->is_key_up(Key::Keyboard::W)) {
			printf("Up\n");
		}

		
		engine.update();
		cam.fly(engine.get_delta_time());
		cam.update();

	

		Vec3 rotation = trans.get_rotation();
		Vec3 rotation2 = trans2.get_rotation();
		//trans.rotateY(rotation.y + 5.0f * (1.0f / 5000.0f));
		//trans.rotateY(rotation.y + 2.0f * engine.get_delta_time());
		//trans2.rotateY(rotation2.y + 50.0f * engine.get_delta_time());

		trans.update();
		trans2.update();

		
		render.render_forward(&trans, engine.get_default_albedo_texture());
		//render.render_forward(&trans2);
		
		win.end_frame();
		trans.force_update();
		trans2.force_update();

		auto start = std::chrono::high_resolution_clock::now();
		bool done = false;
		do {
			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - start;
			done = elapsed >= std::chrono::milliseconds(16);
		} while (!done);

	}

	engine.release();

	return 0;
}

