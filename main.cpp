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
	Scene scene;
	Entity cube = scene.m_ecs.create_entity("cube jou");
	Entity sphere_red = scene.m_ecs.create_entity("Esfera red");
	Entity sphere_ent = scene.m_ecs.create_entity("Esfera");
	Entity cone = scene.m_ecs.create_entity("cono");
	Entity sponza = scene.m_ecs.create_entity("Sponza");
	TransformComponent& t = scene.m_ecs.add_component<TransformComponent>(cube);
	auto& mesh = scene.m_ecs.add_component<MeshComponent>(cube);

	t.set_position({ -5.0f, 4.0f, 0.0f });
	t.set_scale({ scale,scale,scale });

	TransformComponent& t2 = scene.m_ecs.add_component<TransformComponent>(sphere_ent);
	TransformComponent& t_sphere_red = scene.m_ecs.add_component<TransformComponent>(sphere_red);
	t2.set_position({ 0.0f, 3.0f, 0.0f });
	t_sphere_red.set_position({ 0.0f, 6.0f, 0.0f });

	MeshComponent& sphere = scene.m_ecs.add_component<MeshComponent>(sphere_ent);
	MeshComponent& sphere_red_mesh = scene.m_ecs.add_component<MeshComponent>(sphere_red);
	sphere.set_model(engine->get_sphere_high());
	sphere_red_mesh.set_model(engine->get_sphere_high());
	
	Material pbr;
	pbr.set_texture_albedo(engine->m_resource.load_texture("data/materials/rounded_metal/albedo.png"));
	pbr.set_texture_normal(engine->m_resource.load_texture("data/materials/rounded_metal/normal.png"));
	pbr.set_texture_roughness(engine->m_resource.load_texture("data/materials/rounded_metal/roughness.png"));
	pbr.set_texture_metallic(engine->m_resource.load_texture("data/materials/rounded_metal/metallic.png"));
	pbr.set_texture_ao(engine->m_resource.load_texture("data/materials/rounded_metal/ao.png"));
	sphere.set_material(pbr);
	
	Material pbr_red;
	pbr_red.set_texture_albedo(engine->m_resource.load_texture("data/materials/rusted_metal/albedo.png"));
	pbr_red.set_texture_normal(engine->m_resource.load_texture("data/materials/rusted_metal/normal.png"));
	pbr_red.set_texture_roughness(engine->m_resource.load_texture("data/materials/rusted_metal/roughness.png"));
	pbr_red.set_texture_metallic(engine->m_resource.load_texture("data/materials/rusted_metal/metallic.png"));
	pbr_red.set_texture_ao(engine->m_resource.load_texture("data/materials/rusted_metal/ao.png"));
	sphere_red_mesh.set_material(pbr_red);

	

	//t2.set_position({ 2.0f, 4.0f, 5.0f });
	//t2.set_scale({ scale,scale,scale });

	TransformComponent& tcone = scene.m_ecs.add_component<TransformComponent>(cone);
	MeshComponent& mesh_cone = scene.m_ecs.add_component<MeshComponent>(cone);
	mesh_cone.set_model(engine->get_cylinder_high());
	tcone.set_position({ 5.0f, 4.0f, 0.0f });
	tcone.set_scale({ scale,scale,scale });

	TransformComponent& t3 = scene.m_ecs.add_component<TransformComponent>(sponza);
	MeshComponent& meshCompSponza = scene.m_ecs.add_component<MeshComponent>(sponza);
	t3.set_position({ 0.0f, 0.0f, 0.0f });
	t3.set_scale({ scale, scale, scale });

	engine->m_resource.load_mesh_async("data/models/Sponza/Sponza_new.fbx", &meshCompSponza);

	Entity directional_light = scene.m_ecs.create_entity("Directional Light");
	auto& light = scene.m_ecs.add_component<DirectionalLight>(directional_light);
	light.set_color({ 1.0f, 1.0f, 1.0f });
	light.set_direction({ 0.5f,-1.0f, 1.0f });
	light.set_enabled(false);

	Entity point_light = scene.m_ecs.create_entity("Point Light");
	auto& point = scene.m_ecs.add_component<PointLight>(point_light);
	point.set_color({1.0f, 1.0f, 1.0f});
	point.set_position({0.0f, 5.0f, 0.0f});
	point.set_enabled(true);
	point.set_distance(10.0f);

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

