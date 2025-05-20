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

	float scale = 1.0f;


	render.set_camera(&cam);

	Scene scene;
	Entity cube = scene.m_ecs.create_entity("cube jou");
	Entity cube2 = scene.m_ecs.create_entity("cube raro");
	Entity cone = scene.m_ecs.create_entity("cono");
	Entity sponza = scene.m_ecs.create_entity("Sponza");
	TransformComponent& t = scene.m_ecs.add_component<TransformComponent>(cube);
	auto& mesh = scene.m_ecs.add_component<MeshComponent>(cube);
	//mesh.add_material();

	t.set_position({ -2.0f, 4.0f, 0.0f });
	t.set_scale({ scale,scale,scale });

	TransformComponent& t2 = scene.m_ecs.add_component<TransformComponent>(cube2);
	MeshComponent& sphere = scene.m_ecs.add_component<MeshComponent>(cube2);
	sphere.set_model(engine->get_sphere());
	t2.set_position({ 2.0f, 4.0f, 5.0f });
	t2.set_scale({ scale,scale,scale });

	TransformComponent& tcone = scene.m_ecs.add_component<TransformComponent>(cone);
	MeshComponent& mesh_cone = scene.m_ecs.add_component<MeshComponent>(cone);
	mesh_cone.set_model(engine->get_cylinder_high());
	tcone.set_position({ 2.0f, 4.0f, 0.0f });
	tcone.set_scale({ scale,scale,scale });

	TransformComponent& t3 = scene.m_ecs.add_component<TransformComponent>(sponza);
	MeshComponent& meshCompSponza = scene.m_ecs.add_component<MeshComponent>(sponza);
	t3.set_position({ 0.0f, 0.0f, 0.0f });
	t3.set_scale({ scale, scale, scale });

	//Model* model = engine->get_cone();
	Model* model = engine->m_resource.load_mesh("data/models/Sponza/Sponza_new.fbx", true);
	meshCompSponza.set_model(model);

	Entity directional_light = scene.m_ecs.create_entity("Directional Light");
	auto& light = scene.m_ecs.add_component<DirectionalLight>(directional_light);
	light.set_color({ 1.0f, 1.0f, 1.0f });
	light.set_direction({ 0.5f,-1.0f, 1.0f });

	Entity point_light = scene.m_ecs.create_entity("Point Light");
	auto& point = scene.m_ecs.add_component<PointLight>(point_light);
	point.set_color({1.0f, 0.1f, 0.1f});
	point.set_position({0.0f, 5.0f, 0.0f});
	point.set_enabled(false);

	Entity spot_light = scene.m_ecs.create_entity("Spot Light");
	auto& spot = scene.m_ecs.add_component<SpotLight>(spot_light);
	spot.set_color({0.1f, 1.0f, 0.1f});
	spot.set_position({0.0f, 4.0f, -8.0f});
	spot.set_direction({0.0f, 0.0f, 1.0f});
	spot.set_enabled(false);

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

		TransformComponent* t = scene.m_ecs.get_component<TransformComponent>(cube);
		float dt = engine->get_delta_time() * 2.0f;
		//t->rotateXYZ(dt,dt,dt);

		
		scene.update();
		render.render_forward(scene.m_ecs);
		
		win.end_frame();
	}

	engine->release();

	return 0;
}

