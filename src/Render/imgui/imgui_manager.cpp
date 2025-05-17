#include "render/imgui/imgui_manager.h"
#include "Core/engine.h"



ImguiManager::ImguiManager() : m_viewport(nullptr){
	
	m_update_fps_time = 0.25f;
	m_last_fps = 0.0f;
	m_time = 0.0f;
	m_dt_history_size = 128;
	m_dt_history.resize(m_dt_history_size);
	m_dt_history_index = 0;
}

ImguiManager::ImguiManager(const ImguiManager&){

}

ImguiManager::ImguiManager(ImguiManager&&)
{
}

ImguiManager::~ImguiManager(){
#ifdef ENABLE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif 
}

void ImguiManager::begin_frame(){
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_viewport = ImGui::GetMainViewport();

	// Llamar al menu principal, que contiene el dockspace
}

void ImguiManager::end_frame(){

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	ImGuiIO& io = ImGui::GetIO();

	// Render
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

	}
	ImGui::EndFrame();
}

void ImguiManager::render() {

	ImGuiIO& io = ImGui::GetIO();
	
	main_menu();

	/*
	ImGui::Begin("Scene");

	static float my_color[4];
	ImGui::ColorEdit4("Color", my_color);
	ImGui::End();
	*/

	system_info();
	


	// DEMO
	//static bool show = true;
	//ImGui::ShowStyleEditor();
	//ImGui::ShowDemoWindow();

	
}

void ImguiManager::resize(HWND hwnd,unsigned int width, unsigned int height){
	ImGuiIO& io = ImGui::GetIO();
	RECT rect;
	GetClientRect(hwnd, &rect);
	//io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	io.DisplaySize = ImVec2(width, height);


	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void ImguiManager::apply_nova_style(){
	ImGuiStyle& style = ImGui::GetStyle();

	// Tamaños y espaciados
	style.FrameRounding = 3.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.FramePadding = ImVec2(15.0f, 5.0f);
	style.ItemSpacing = ImVec2(12.0f, 5.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 3.0f); // 3 es el valor por defecto
	style.ScrollbarSize = 17.0f;
	style.GrabMinSize = 13.0f;

	// Bordes y redondeos
	style.WindowRounding = 2.0f;
	style.TabBorderSize = 2.0f;
	style.TabRounding = 3.0f;

	// Configuración avanzada
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f); // MenuButtonPosition = Right
	style.WindowMenuButtonPosition = ImGuiDir_Right;

	style.SeparatorTextBorderSize = 3.0f;
	style.SeparatorTextAlign = ImVec2(0.03f, 0.5f);

	style.WindowBorderSize = 1.0f; // Necesario para que el hover padding se note
	style.WindowBorderSize = 1.0f;
	style.WindowBorderHoverPadding = 4.0f;

	// Docking separator
	style.DockingSeparatorSize = 5.0f; // Solo disponible con Docking Branch
}

void ImguiManager::show_demo_window(){


	
}

void ImguiManager::init(HWND handle){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplWin32_Init(handle);
	ImGui_ImplDX11_Init(Engine::get_instance()->get_engine_props()->deviceInterface, Engine::get_instance()->get_engine_props()->inmediateDeviceContext);

	apply_nova_style();
}

void ImguiManager::main_menu(){
	ImGui::SetNextWindowSize(ImVec2(m_viewport->Size.x, m_viewport->Size.y));
	ImGui::SetNextWindowPos(ImVec2(m_viewport->Pos.x, m_viewport->Pos.y));
	ImGui::SetNextWindowViewport(m_viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::Begin("main-window", nullptr, window_flags);

	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	//ImGuiDockNodeFlags_NoDockingInCentralNode add this
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode); //|ImGuiDockNodeFlags_AutoHideTabBar  | ImGuiDockNodeFlags_NoResize


	// Menu buttons
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
	if (ImGui::BeginMenuBar()) {
		if(ImGui::MenuItem("Scene", "shortcut")){
			
		}
		
		
		
		ImGui::EndMenuBar();
	}
	ImGui::PopStyleVar();

	ImGui::End();
}

void ImguiManager::system_info(){

	Engine* e = Engine::get_instance();
	float mouse_x = e->m_input.get_mouse_x();
	float mouse_y = e->m_input.get_mouse_y();
	
	float fps = e->get_fps();
	float dt = e->get_delta_time();




	m_time += dt;

	if (m_time >= m_update_fps_time) {
		// Update plot lines
		m_dt_history[m_dt_history_index] = dt;
		m_dt_history_index++;
		m_dt_history_index %= m_dt_history_size;

		// Update FPS
		m_time -= m_update_fps_time;
		m_last_fps = fps;
	}

	ImGui::Begin("System info");

	ImGui::Text("FPS: %f", m_last_fps);
	ImGui::Text("Delta Time: %f", dt);
	ImGui::PlotLines("Delta time", m_dt_history.data(), m_dt_history_size, m_dt_history_index, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 70));

	ImGui::Text("Mouse (%.0f, %.0f)", mouse_x, mouse_y);




	ImGui::End();
}

void ImguiManager::show_transform(TransformComponent* trans, int entity_id){

	ImGui::SeparatorText("Transform");
	Vec3& pos = trans->get_position();
	Vec3& rotation = trans->get_rotation();
	Vec3& scale = trans->get_scale();
	float tmp_pos[3] = { pos.x, pos.y, pos.z };
	float tmp_rot[3] = { rotation.x, rotation.y, rotation.z };
	float tmp_scale[3] = { scale.x, scale.y, scale.z };

	std::string label = "Position##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), tmp_pos, 0.1f);

	label = "Rotation##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), tmp_rot, 0.1f);

	label = "Scale##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), tmp_scale, 0.1f);



	trans->set_position({ tmp_pos[0], tmp_pos[1], tmp_pos[2]});
	trans->set_rotation({ tmp_rot[0], tmp_rot[1], tmp_rot[2]});
	trans->set_scale({ tmp_scale[0], tmp_scale[1], tmp_scale[2]});

}

void ImguiManager::show_directional(DirectionalLight* light, int entity_id){
	
	ImGui::SeparatorText("Directional Light");

	Vec3& direction = light->get_direction();
	float dir_tmp[3] = { direction.x, direction.y, direction.z };

	bool enabled = light->get_enabled();

	Vec3& color =  light->get_color();
	float color_tmp[3] = {color.x, color.y, color.z};

	float specular_strenght = light->get_specular_strenght();

	Vec3& specular_color = light->get_specular_color();
	float spec_color_tmp[3] = {specular_color.x, specular_color.y, specular_color.z };
	
	float specular_shininess = light->get_specular_shininess();

	std::string label = "Direction##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), dir_tmp, 0.005f, -1.0f, 1.0f);

	label = "Enabled##" + std::to_string(entity_id);
	ImGui::Checkbox(label.c_str(), &enabled);

	label = "Color##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), color_tmp, 0.005f, 0.0f, 1.0f);

	label = "Specular Strenght##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &specular_strenght, 0.001f, 0.0f, 1.0f);

	label = "Specular Color##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), spec_color_tmp, 0.005f, -1.0f, 1.0f);

	label = "Specular Shininess##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &specular_shininess);

	if (dir_tmp[0] != 0.5f) {
		printf("diablo");
	}
	light->set_direction({ dir_tmp[0], dir_tmp[1], dir_tmp[2] });
	light->set_enabled(enabled);
	light->set_color({color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strenght);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);


}

void ImguiManager::show_cam(CameraComponent* cam, int entity_id){
	ImGui::Begin("Prueba");

	ImGui::SeparatorText("Camera");
	Vec3 position = cam->get_position();
	float position_tmp[3] = {position.x, position.y, position.z};

	float near_t = cam->get_near();
	float far_t = cam->get_far();
	float pitch = cam->get_pitch();
	float yaw = cam->get_yaw();
	float speed = cam->get_speed();
	float mov_speed = cam->get_movement_speed();
	float sensitivity = cam->get_mouse_sensitivity();

	std::string label = "Position##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), position_tmp, 0.1f);

	label = "Near##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &near_t, 0.001f, 0.001f, 1.0f);

	label = "Far##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &far_t, 1.0f, 10000.0f, 1.0f);
	
	label = "Pitch##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &pitch, 1.0f);

	label = "Yaw##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &yaw, 1.0f);
	
	label = "Speed##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &speed, 1.0f, 0.0f, 1000.0f);

	label = "Movement speed##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &mov_speed, 1.0f, 0.0f, 1000.0f);
	
	label = "Mouse sensitivity##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &sensitivity, 0.1f, 0.0f, 1000.0f);

	cam->set_position({position_tmp[0], position_tmp[1], position_tmp[2]});
	cam->set_near(near_t);
	cam->set_far(far_t);
	cam->set_pitch(pitch);
	cam->set_yaw(yaw);
	cam->set_speed(speed);
	cam->set_movement_speed(mov_speed);
	cam->set_mouse_sensitivity(sensitivity);

	ImGui::End();

}

void ImguiManager::scene_info(EntityComponentSystem& ecs){

	std::vector<Entity>& entities = ecs.get_entities();

	ImGui::Begin("Scene");
	for (auto e : entities) {

		//ImGui::SeparatorText(name.c_str());
		std::string name = e.get_name() + "## " + std::to_string(e.get_id());
		if (ImGui::CollapsingHeader(name.c_str())) {

			std::string id_text = "ID: " + std::to_string(e.get_id());
			ImGui::Text(id_text.c_str());
			TransformComponent* trans = ecs.get_component<TransformComponent>(e);
			if (trans) {
				show_transform(trans, e.get_id());
			}

			DirectionalLight* dir_light = ecs.get_component<DirectionalLight>(e);
			if (dir_light) {
				show_directional(dir_light, e.get_id());
			}
		}
	}
	ImGui::End();
}



