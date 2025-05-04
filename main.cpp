#include "nova_engine.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	
	Engine* engine = Engine::get_instance();;
	
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

	engine->init_geometries();

	CameraComponent cam(engine->get_input(), &win);

	float scale = 1.0f;


	render.set_camera(&cam);

	Scene scene;
	Entity cube = scene.m_ecs.create_entity();
	Entity cube2 = scene.m_ecs.create_entity();
	Entity sponza = scene.m_ecs.create_entity();
	TransformComponent& t = scene.m_ecs.add_component<TransformComponent>(cube);
	MaterialComponent& mat = scene.m_ecs.add_component<MaterialComponent>(cube);
	scene.m_ecs.add_component<MeshComponent>(cube);

	t.set_position({ -2.0f, 4.0f, 10.0f });
	t.set_scale({scale,scale,scale});
	
	TransformComponent& t2 = scene.m_ecs.add_component<TransformComponent>(cube2);
	MaterialComponent& mat2 = scene.m_ecs.add_component<MaterialComponent>(cube2);
	scene.m_ecs.add_component<MeshComponent>(cube2);
	t2.set_position({ 2.0f, 4.0f, 10.0f });
	t2.set_scale({scale,scale,scale});
	
	TransformComponent& t3 = scene.m_ecs.add_component<TransformComponent>(sponza);
	MaterialComponent& mat3 = scene.m_ecs.add_component<MaterialComponent>(sponza);
	MeshComponent& meshCompSponza = scene.m_ecs.add_component<MeshComponent>(sponza);
	t3.set_position({ 0.0f, 0.0f, 0.0f });
	t3.set_scale({0.05f,0.05f,0.05f });

	Model* model = engine->m_resource.load_mesh("data/models/Sponza/Sponza.fbx", &mat3);
	meshCompSponza.set_model(model);


	while (true) {

		win.begin_frame();
		// if true no execute game code?
		if (!win.update()) {
			printf("window update return false\n");
			break;
		}
		//printf("hola");
		std::string dt_s = std::string("DT: ") + std::to_string(engine->get_delta_time());
		std::string fps_s = std::string("FPS: ") + std::to_string(engine->get_fps());
		//OutputDebugStringA(dt_s.c_str());
		//OutputDebugStringA(fps_s.c_str());
		//6printf("%s\n", fps_s.c_str());

		
		engine->update();
		cam.fly(engine->get_delta_time());
		cam.update();


	
		TransformComponent* t = scene.m_ecs.get_component<TransformComponent>(cube);
		float dt = engine->get_delta_time() * 2.0f;
		t->rotateXYZ(dt,dt,dt);

		
		scene.update();
		render.render_forward(scene.m_ecs);
		//render.render_forward(&trans2);
		
		win.end_frame();

		/*
		auto start = std::chrono::high_resolution_clock::now();
		bool done = false;
		do {
			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - start;
			done = elapsed >= std::chrono::milliseconds(16);
		} while (!done);
		*/

	}

	engine->release();

	return 0;
}

