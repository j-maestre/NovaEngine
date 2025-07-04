#include "nova_engine.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {


	Engine* engine = Engine::get_instance();

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

	engine->open_console();
	win.init(&props);
	engine->init(&win);

	Renderer render;
	bool ret = render.init_pipeline(&win);
	win.init_imgui();

	engine->init_geometries();

	CameraComponent cam(engine->get_input(), &win);

	render.set_camera(&cam);


	float scale = 1.0f;
	Scene scene("scene0.yaml");
	
	Entity directional_light = scene.m_ecs.create_entity("Directional Light");
	auto& light = scene.m_ecs.add_component<DirectionalLight>(directional_light);
	light.set_color({ 1.0f, 1.0f, 1.0f });
	light.set_direction({ -0.5f,-1.0f, -1.0f });
	light.set_enabled(true);

	/*
	Entity point_light = scene.m_ecs.create_entity("Point Light");
	auto& point = scene.m_ecs.add_component<PointLight>(point_light);
	point.set_color({1.0f, 1.0f, 1.0f});
	point.set_position({0.0f, 5.0f, 0.0f});
	point.set_enabled(false);
	point.set_distance(10.0f);
	*/

	Entity spot_light = scene.m_ecs.create_entity("Spot Light");
	auto& spot = scene.m_ecs.add_component<SpotLight>(spot_light);
	spot.set_color({1.0f, 1.0f, 1.0f});
	spot.set_position({0.0f, 4.0f, -7.0f});
	spot.set_direction({0.0f, 0.0f, 1.0f});
	spot.set_enabled(false);
	spot.set_distance(15.0f);

	//win.set_full_screen();

	while (true) {

		win.begin_frame();

		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}

		engine->update();

		cam.fly(engine->get_delta_time());

		cam.update();

		
		scene.update();
		//render.render_forward(scene.m_ecs);
		render.render_deferred(scene.m_ecs);

		win.end_frame();
		
	}

	engine->release();

	return 0;
}

