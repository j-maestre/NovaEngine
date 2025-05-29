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
	io.DisplaySize = ImVec2((float)width, (float)height);


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
	int mouse_x = e->m_input.get_mouse_x();
	int mouse_y = e->m_input.get_mouse_y();
	
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

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
	ImGui::Text("FPS: %.0f", m_last_fps);
	ImGui::PopStyleColor();

#ifdef MEASURE_TIME
	ImGui::Text("Delta Time: %f", dt);
	ImGui::Text("Draw Time: %f", m_draw_time);
	ImGui::Text("Draw ImGui Time: %f", m_draw_imgui_time);
	ImGui::Text("Update transform time: %f", m_update_transform_time);
	ImGui::Text("Window begin frame time: %f", m_window_begin_frame_time);
	ImGui::Text("Window end frame time: %f", m_window_end_frame_time);
	ImGui::Text("Engine update time: %f", m_engine_update_time);
	ImGui::Text("Update cam time: %f", m_update_cam_time);
	ImGui::Text("Fly cam time: %f", m_fly_cam_time);
	ImGui::PlotLines("Delta time", m_dt_history.data(), m_dt_history_size, m_dt_history_index, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 70));
#endif

	ImGui::Checkbox("Show Bloom", &m_bloom);

	ImGui::Text("Mouse (%d, %d)", mouse_x, mouse_y);
	ImGui::Text("Delta (%.0f, %.0f)", m_mouse_delta_x, m_mouse_delta_y);
	
	bool left_click = Engine::get_instance()->m_input.is_key_pressed(Key::Mouse::LBUTTON);
	bool right_click = Engine::get_instance()->m_input.is_key_pressed(Key::Mouse::RBUTTON);
	ImGui::Text("Click (%d,%d)", left_click?1:0, right_click?1:0);

	ImGui::SeparatorText("Resources loaded");
	if (ImGui::Button("Clear")) {
		m_resources_loaded.clear();
	}

	for (auto& s : m_resources_loaded) {
		ImGui::Text(s.c_str());
	}



	ImGui::End();
}


void ImguiManager::show_transform(TransformComponent* trans, int entity_id){

	ImGui::SeparatorText("Transform");

	Vec3 pos = trans->get_position();
	Vec3 rot = trans->get_rotation();
	Vec3 scl = trans->get_scale();

	rot.x = radToDeg(rot.x);
	rot.y = radToDeg(rot.y);
	rot.z = radToDeg(rot.z);

	// Calcular el ancho máximo para las etiquetas
	const char* labels[] = { "Position", "Rotation", "Scale" };
	float max_label_width = 0.0f;
	for (const char* lbl : labels) {
		float w = ImGui::CalcTextSize(lbl).x;
		if (w > max_label_width) max_label_width = w;
	}
	max_label_width += 10.0f; // margen extra para que no se pegue el texto a los controles

	auto draw_vec3_control = [&](const char* label, Vec3& vec) {
		ImGui::PushID(label);

		ImGui::AlignTextToFramePadding();

		// 1. Crear espacio reservado para la etiqueta
		ImGui::BeginGroup();
		ImGui::TextUnformatted(label);

		// 2. Mover el cursor hacia la derecha, al final del espacio reservado
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(max_label_width - ImGui::CalcTextSize(label).x, 0)); // espacio vacío para que la siguiente línea empiece bien alineada
		ImGui::SameLine();

		// Ahora el cursor está justo donde queremos empezar los controles (botones y sliders)

		const char* axes[3] = { "X", "Y", "Z" };
		ImVec4 colors[3] = {
			ImVec4(0.8f, 0.1f, 0.15f, 1.0f),
			ImVec4(0.2f, 0.7f, 0.2f, 1.0f),
			ImVec4(0.1f, 0.25f, 0.8f, 1.0f)
		};

		float* components[3] = { &vec.x, &vec.y, &vec.z };
		float button_width = 18.0f;
		float button_height = 18.0f;
		float spacing = ImGui::GetStyle().ItemSpacing.x;
		float text_height = ImGui::GetTextLineHeight();
		float vertical_offset = (text_height - button_height) * 0.5f;

		// Calcular ancho restante para sliders
		float total_button_area = (button_width + spacing + 2.0f) * 3;
		float total_available = ImGui::GetContentRegionAvail().x;
		float slider_width = (total_available - total_button_area) / 3.0f;

		float line_start_y = ImGui::GetCursorPosY();

		for (int i = 0; i < 3; ++i) {
			ImGui::SetCursorPosY(line_start_y - vertical_offset);

			// Botón coloreado
			std::string button_id = std::string("##") + label + "_btn_" + axes[i] + "_" + std::to_string(entity_id);
			ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
			ImGui::Button(button_id.c_str(), ImVec2(button_width, button_height));
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}

			// Centrar texto en el botón
			ImVec2 btn_min = ImGui::GetItemRectMin();
			ImVec2 btn_size = ImGui::GetItemRectSize();
			ImVec2 text_size = ImGui::CalcTextSize(axes[i]);
			ImVec2 text_pos = {
				btn_min.x + (btn_size.x - text_size.x) * 0.5f,
				btn_min.y + (btn_size.y - text_size.y) * 0.5f
			};
			ImGui::GetWindowDrawList()->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), axes[i]);

			if (ImGui::IsItemClicked()) {
				*components[i] = 0.0f;
			}

			ImGui::SameLine(0.0f, 2.0f);

			// Slider dinámico
			std::string drag_id = std::string("##") + label + "_drag_" + axes[i] + "_" + std::to_string(entity_id);
			ImGui::SetNextItemWidth(slider_width);
			ImGui::DragFloat(drag_id.c_str(), components[i], 0.1f);

			if (i < 2)
				ImGui::SameLine();
		}

		ImGui::EndGroup();

		ImGui::PopID();
		};

	draw_vec3_control("Position", pos);
	draw_vec3_control("Rotation", rot);
	draw_vec3_control("Scale", scl);

	rot.x = degToRad(rot.x);
	rot.y = degToRad(rot.y);
	rot.z = degToRad(rot.z);


	trans->set_position(pos);
	trans->set_rotation(rot);
	trans->set_scale(scl);
}

void ImguiManager::show_light(PointLight* light, int entity_id){
	ImGui::SeparatorText("Point Light");

	Vec3 position = light->get_position();
	float pos_tmp[3] = { position.x, position.y, position.z };


	bool enabled = light->get_enabled();

	Vec3 color = light->get_color();
	float color_tmp[3] = { color.x, color.y, color.z };

	float specular_strenght = light->get_specular_strenght();

	Vec3 specular_color = light->get_specular_color();
	float spec_color_tmp[3] = { specular_color.x, specular_color.y, specular_color.z };

	float specular_shininess = light->get_specular_shininess();

	float constant_att = light->get_constant_attenuation();
	float linear_att = light->get_linear_attenuation();
	float cuadratic_att = light->get_quadratic_attenuation();
	float att = light->get_attenuation();
	float intensity = light->get_intensity();
	float expossure = light->get_expossure();
	float distance = light->get_distance();
	float fall_off = light->get_fall_off();
	float fall_start = light->get_fall_start();


	std::string label = "Position##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), pos_tmp, 0.1f);

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

	label = "Atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &att, 0.0001f, 0.0f);

	label = "Constant atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &constant_att, 0.0001f, 0.0f);

	label = "Linear atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &linear_att, 0.0001f, 0.0f);

	label = "Cuadratic atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &cuadratic_att, 0.0001f, 0.0f);

	label = "Intensity##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &intensity, 0.1f, 0.0f, 10000.0f);

	label = "Expossure##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &expossure, 0.1f, 0.0f, 200.0f);

	label = "Distance##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &distance, 0.1f, 0.0f, 20000.0f);

	label = "Fall off##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &fall_off, 0.1f, 0.0f, 10000.0f);
	
	label = "Fall start##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &fall_start, 0.01f, 0.0f, 1.0f);

	light->set_position({pos_tmp[0], pos_tmp[1], pos_tmp[2] });
	light->set_enabled(enabled);
	light->set_color({ color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strenght);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);
	light->set_attenuation(att);
	light->set_constant_attenuation(constant_att);
	light->set_linear_attenuation(linear_att);
	light->set_quadratic_attenuation(cuadratic_att);
	light->set_intensity(intensity);
	light->set_expossure(expossure);
	light->set_distance(distance);
	light->set_fall_off(fall_off);
	light->set_fall_start(fall_start);
}

void ImguiManager::show_light(SpotLight* light, int entity_id){
	ImGui::SeparatorText("Spot Light");

	Vec3 position = light->get_position();
	float pos_tmp[3] = { position.x, position.y, position.z };

	Vec3 direction = light->get_direction();
	float dir_tmp[3] = { direction.x, direction.y, direction.z };

	bool enabled = light->get_enabled();

	Vec3 color = light->get_color();
	float color_tmp[3] = { color.x, color.y, color.z };

	float specular_strenght = light->get_specular_strenght();

	Vec3 specular_color = light->get_specular_color();
	float spec_color_tmp[3] = { specular_color.x, specular_color.y, specular_color.z };

	float cut_off = light->get_inner_circle();
	float outer_cut_off = light->get_outside_circle();
	float specular_shininess = light->get_specular_shininess();
	float constant_att = light->get_constant_attenuation();
	float linear_att = light->get_linear_attenuation();
	float cuadratic_att = light->get_quadratic_attenuation();
	float expossure = light->get_expossure();
	float distance = light->get_distance();
	float intensity = light->get_intensity();


	std::string label = "Position##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), pos_tmp, 0.1f);
	
	label = "Direction##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), dir_tmp, 0.1f);

	label = "Enabled##" + std::to_string(entity_id);
	ImGui::Checkbox(label.c_str(), &enabled);

	label = "Inner circle##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &cut_off, 0.01f, 0.0f);

	label = "Outside circle##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &outer_cut_off, 0.01f, 0.0f);

	label = "Color##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), color_tmp, 0.005f, 0.0f, 1.0f);

	label = "Specular Strenght##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &specular_strenght, 0.001f, 0.0f, 1.0f);

	label = "Specular Color##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), spec_color_tmp, 0.005f, -1.0f, 1.0f);

	label = "Specular Shininess##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &specular_shininess);

	label = "Constant atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &constant_att, 0.0001f, 0.0f);

	label = "Linear atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &linear_att, 0.0001f, 0.0f);

	label = "Cuadratic atenuattion##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &cuadratic_att, 0.0001f, 0.0f);

	label = "Expossure##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &expossure, 0.1f, 0.0f, 200.0f);

	label = "Distance##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &distance, 0.1f, 0.0f, 2000.0f);

	label = "Intensity##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &intensity, 0.1f, 0.0f, 10000.0f);

	light->set_position({ pos_tmp[0], pos_tmp[1], pos_tmp[2] });
	light->set_direction({ dir_tmp[0], dir_tmp[1], dir_tmp[2] });
	light->set_inner_circle(cut_off);
	light->set_outer_circle(outer_cut_off);
	light->set_enabled(enabled);
	light->set_color({ color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strenght);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);
	light->set_constant_attenuation(constant_att);
	light->set_linear_attenuation(linear_att);
	light->set_quadratic_attenuation(cuadratic_att);
	light->set_expossure(expossure);
	light->set_distance(distance);
	light->set_intensity(intensity);

}

void ImguiManager::show_mesh_comp(MeshComponent* mesh_comp, int entity_id){

	
	ImGui::SeparatorText("Material");
	int index = 0;
	for (auto& m : mesh_comp->get_model()->meshes) {

		std::string label = "Mesh " + std::to_string(index);
		ImGui::Text(label.c_str());
		Vec4 color_v = m.material.get_color_value();
		Vec3 color_emissive = m.material.get_emissive_value();
		float color[3] = {color_v.x, color_v.y,color_v.z};
		float emissive[3] = { color_emissive.x, color_emissive.y,color_emissive.z };

		float metallic = m.material.get_metallic_value();
		float roughness = m.material.get_roughness_value();

		ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

		label = "Color##" + std::to_string(entity_id) + std::to_string(index);
		ImGui::ColorEdit3(label.c_str(), color, flags);
		
		label = "Emissive##" + std::to_string(entity_id) + std::to_string(index);
		ImGui::ColorEdit3(label.c_str(), emissive, flags);
		
		label = "Metallic##" + std::to_string(entity_id) + std::to_string(index);
		ImGui::DragFloat(label.c_str(), &metallic, 0.01f, 0.0f, 1.0f);
		
		label = "Roughness##" + std::to_string(entity_id) + std::to_string(index);
		ImGui::DragFloat(label.c_str(), &roughness, 0.01f, 0.0f, 1.0f);
		

		m.material.set_color_value({color[0],color[1], color[2], color[3] });
		m.material.set_emissive_value({ emissive[0],emissive[1], emissive[2] });
		m.material.set_metallic_value(metallic);
		m.material.set_roughness_value(roughness);
		index++;
	}
	
}

void ImguiManager::show_light(DirectionalLight* light, int entity_id){
	
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

	float intensity = light->get_intensity();

	std::string label = "Direction##" + std::to_string(entity_id);
	ImGui::DragFloat3(label.c_str(), dir_tmp, 0.005f, -1.0f, 1.0f);

	label = "Intensity##" + std::to_string(entity_id);
	ImGui::DragFloat(label.c_str(), &intensity, 0.01f, 0.0f);

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
	light->set_intensity(intensity);


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

void ImguiManager::add_resource_loaded(std::string text){

	std::lock_guard<std::mutex> locked{ resources_lock };
	m_resources_loaded.push_back(text);
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
				show_light(dir_light, e.get_id());
			}
			
			PointLight* point_light = ecs.get_component<PointLight>(e);
			if (point_light) {
				show_light(point_light, e.get_id());
			}
			
			SpotLight* spot_light = ecs.get_component<SpotLight>(e);
			if (spot_light) {
				show_light(spot_light, e.get_id());
			}

			
			MeshComponent* mesh_comp = ecs.get_component<MeshComponent>(e);
			if (mesh_comp) {
				show_mesh_comp(mesh_comp, e.get_id());
			}
			
		}

		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
	}
	ImGui::End();
}



