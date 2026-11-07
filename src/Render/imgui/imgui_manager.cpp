#include "render/imgui/imgui_manager.h"
#include "Core/engine.h"
#include "imgui_internal.h"
#include <filesystem>
#include <algorithm>
#include <string>
#include <functional>
#include <unordered_map>


namespace {

	// Paleta de la UI
	const ImVec4 k_bg(0.086f, 0.094f, 0.114f, 1.0f);
	const ImVec4 k_panel(0.110f, 0.122f, 0.149f, 1.0f);
	const ImVec4 k_frame(0.165f, 0.180f, 0.216f, 1.0f);
	const ImVec4 k_frame_hover(0.220f, 0.239f, 0.286f, 1.0f);
	const ImVec4 k_frame_active(0.263f, 0.286f, 0.341f, 1.0f);
	const ImVec4 k_border(0.200f, 0.216f, 0.255f, 1.0f);
	const ImVec4 k_text(0.890f, 0.906f, 0.937f, 1.0f);
	const ImVec4 k_text_dim(0.580f, 0.612f, 0.671f, 1.0f);
	const ImVec4 k_accent(0.357f, 0.549f, 1.000f, 1.0f);
	const ImVec4 k_accent_hover(0.463f, 0.631f, 1.000f, 1.0f);
	const ImVec4 k_accent_soft(0.357f, 0.549f, 1.000f, 0.28f);
	const ImVec4 k_green(0.361f, 0.839f, 0.553f, 1.0f);
	const ImVec4 k_amber(0.973f, 0.749f, 0.325f, 1.0f);
	const ImVec4 k_red(0.961f, 0.420f, 0.420f, 1.0f);

	const unsigned int k_no_entity = static_cast<unsigned int>(-1);

	// Rejilla de propiedades: etiqueta a la izquierda, control a la derecha
	bool begin_props(const char* id) {
		if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) return false;
		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch, 0.62f);
		return true;
	}

	void prop_label(const char* label) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(k_text_dim, "%s", label);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
	}

	std::string hidden_id(const char* label) {
		return std::string("##") + label;
	}

	bool prop_float(const char* label, float* value, float speed, float min = 0.0f, float max = 0.0f, const char* format = "%.2f") {
		prop_label(label);
		return ImGui::DragFloat(hidden_id(label).c_str(), value, speed, min, max, format);
	}

	bool prop_slider(const char* label, float* value, float min, float max, const char* format = "%.2f") {
		prop_label(label);
		return ImGui::SliderFloat(hidden_id(label).c_str(), value, min, max, format);
	}

	bool prop_color(const char* label, float color[3]) {
		prop_label(label);
		return ImGui::ColorEdit3(hidden_id(label).c_str(), color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions);
	}

	bool prop_check(const char* label, bool* value) {
		prop_label(label);
		return ImGui::Checkbox(hidden_id(label).c_str(), value);
	}

	// Tres campos X/Y/Z. Clic en la letra: vuelve al valor de reset.
	bool prop_vec3(const char* label, Vec3& vec, float reset, float speed, const char* format = "%.2f") {
		prop_label(label);
		ImGui::PushID(label);

		const char* axes[3] = { "X", "Y", "Z" };
		const ImVec4 colors[3] = {
			ImVec4(0.800f, 0.255f, 0.290f, 1.0f),
			ImVec4(0.275f, 0.655f, 0.380f, 1.0f),
			ImVec4(0.255f, 0.420f, 0.820f, 1.0f),
		};
		float* components[3] = { &vec.x, &vec.y, &vec.z };

		const float gap = 4.0f;
		const float tag_width = ImGui::GetFrameHeight() * 0.8f;
		const float avail = ImGui::GetContentRegionAvail().x;
		const float drag_width = std::max(10.0f, (avail - tag_width * 3.0f - gap * 2.0f) / 3.0f);

		bool changed = false;
		for (int i = 0; i < 3; i++) {
			ImGui::PushID(i);

			ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colors[i].x * 1.2f, colors[i].y * 1.2f, colors[i].z * 1.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[i]);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			if (ImGui::Button(axes[i], ImVec2(tag_width, 0.0f))) {
				*components[i] = reset;
				changed = true;
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);

			ImGui::SameLine(0.0f, 0.0f);
			ImGui::SetNextItemWidth(drag_width);
			changed |= ImGui::DragFloat("##value", components[i], speed, 0.0f, 0.0f, format);

			if (i < 2) ImGui::SameLine(0.0f, gap);
			ImGui::PopID();
		}

		ImGui::PopID();
		return changed;
	}

	bool section(const char* name, bool default_open = true) {
		return ImGui::CollapsingHeader(name, default_open ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
	}

	void centered_hint(const char* text) {
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const ImVec2 size = ImGui::CalcTextSize(text, nullptr, false, avail.x);
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + std::max(0.0f, (avail.x - size.x) * 0.5f), ImGui::GetCursorPosY() + avail.y * 0.4f));
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + size.x + 1.0f);
		ImGui::TextColored(k_text_dim, "%s", text);
		ImGui::PopTextWrapPos();
	}

	// Filas etiqueta / barra / ms. La barra es la parte de total_ms que se lleva cada una.
	void timing_row(const char* label, float ms, float total_ms) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextColored(k_text_dim, "%s", label);

		ImGui::TableSetColumnIndex(1);
		const float fraction = total_ms > 0.0f ? ms / total_ms : 0.0f;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetTextLineHeight() - 5.0f) * 0.5f);
		ImGui::ProgressBar(std::min(fraction, 1.0f), ImVec2(-FLT_MIN, 5.0f), "");

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%.2f ms", ms);
	}

	bool begin_timing_table(const char* id) {
		if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) return false;
		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthStretch, 0.45f);
		ImGui::TableSetupColumn("bar", ImGuiTableColumnFlags_WidthStretch, 0.35f);
		ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthStretch, 0.20f);
		return true;
	}

	// True si el raton esta sobre la UI. No se usa IsWindowHovered porque devuelve false justo
	// cuando el clic activa un control del panel, ni WantCaptureMouse porque ImGuizmo tambien lo activa.
	bool is_mouse_over_ui() {
		return GImGui->HoveredWindow != nullptr || ImGui::IsAnyItemActive() || ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
	}

	ImVec4 fps_color(float fps) {
		if (fps >= 60.0f) return k_green;
		if (fps >= 30.0f) return k_amber;
		return k_red;
	}
}


ImguiManager::ImguiManager() : m_viewport(nullptr){

	m_update_fps_time = 0.25f;
	m_last_fps = 0.0f;
	m_time = 0.0f;
	m_dt_history_size = 240;
	m_dt_history.resize(m_dt_history_size);
	m_dt_history_index = 0;

	m_draw_modes.push_back("Solid");
	m_draw_modes.push_back("Wireframe");
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

	system_info();

	// Sin atajos mientras se escribe en un campo o se vuela con la camara (W tambien es avanzar)
	Input& input = Engine::get_instance()->m_input;
	if (!io.WantTextInput && !input.is_key_pressed(Key::Mouse::RBUTTON)) {
		if (input.is_key_down(Key::Keyboard::W)) m_current_operation = ImGuizmo::TRANSLATE;
		if (input.is_key_down(Key::Keyboard::E)) m_current_operation = ImGuizmo::SCALE;
		if (input.is_key_down(Key::Keyboard::R)) m_current_operation = ImGuizmo::ROTATE;
	}
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

	// Espaciados
	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.FramePadding = ImVec2(8.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 3.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
	style.IndentSpacing = 14.0f;
	style.ScrollbarSize = 11.0f;
	style.GrabMinSize = 10.0f;

	// Bordes y redondeos. Con viewports las ventanas sueltas son ventanas del sistema: sin redondeo.
	style.WindowRounding = 0.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 4.0f;
	style.PopupRounding = 4.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 6.0f;
	style.TabRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;
	style.WindowBorderHoverPadding = 4.0f;
	style.DockingSeparatorSize = 2.0f;

	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.SeparatorTextBorderSize = 1.0f;
	style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
	style.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesToNodes;
	style.TreeLinesSize = 1.0f;

	ImVec4* c = style.Colors;
	c[ImGuiCol_Text] = k_text;
	c[ImGuiCol_TextDisabled] = k_text_dim;
	c[ImGuiCol_WindowBg] = k_panel;
	c[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	c[ImGuiCol_PopupBg] = k_bg;
	c[ImGuiCol_Border] = k_border;
	c[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	c[ImGuiCol_FrameBg] = k_frame;
	c[ImGuiCol_FrameBgHovered] = k_frame_hover;
	c[ImGuiCol_FrameBgActive] = k_frame_active;
	c[ImGuiCol_TitleBg] = k_bg;
	c[ImGuiCol_TitleBgActive] = k_bg;
	c[ImGuiCol_TitleBgCollapsed] = k_bg;
	c[ImGuiCol_MenuBarBg] = k_bg;
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	c[ImGuiCol_ScrollbarGrab] = k_frame_hover;
	c[ImGuiCol_ScrollbarGrabHovered] = k_frame_active;
	c[ImGuiCol_ScrollbarGrabActive] = k_accent;
	c[ImGuiCol_CheckMark] = k_accent;
	c[ImGuiCol_SliderGrab] = k_accent;
	c[ImGuiCol_SliderGrabActive] = k_accent_hover;
	c[ImGuiCol_Button] = k_frame;
	c[ImGuiCol_ButtonHovered] = k_frame_hover;
	c[ImGuiCol_ButtonActive] = k_frame_active;
	c[ImGuiCol_Header] = k_accent_soft;
	c[ImGuiCol_HeaderHovered] = k_frame_hover;
	c[ImGuiCol_HeaderActive] = k_frame_active;
	c[ImGuiCol_TreeLines] = k_frame_active;
	c[ImGuiCol_DragDropTarget] = k_accent;
	c[ImGuiCol_Separator] = k_border;
	c[ImGuiCol_SeparatorHovered] = k_accent;
	c[ImGuiCol_SeparatorActive] = k_accent;
	c[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	c[ImGuiCol_ResizeGripHovered] = k_accent_soft;
	c[ImGuiCol_ResizeGripActive] = k_accent;
	c[ImGuiCol_Tab] = k_bg;
	c[ImGuiCol_TabHovered] = k_frame_hover;
	c[ImGuiCol_TabSelected] = k_panel;
	c[ImGuiCol_TabSelectedOverline] = k_accent;
	c[ImGuiCol_TabDimmed] = k_bg;
	c[ImGuiCol_TabDimmedSelected] = k_panel;
	c[ImGuiCol_TabDimmedSelectedOverline] = k_border;
	c[ImGuiCol_DockingPreview] = k_accent_soft;
	c[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	c[ImGuiCol_PlotLines] = k_accent;
	c[ImGuiCol_PlotLinesHovered] = k_accent_hover;
	c[ImGuiCol_PlotHistogram] = k_accent;
	c[ImGuiCol_PlotHistogramHovered] = k_accent_hover;
	c[ImGuiCol_TableBorderLight] = k_border;
	c[ImGuiCol_TableBorderStrong] = k_border;
	c[ImGuiCol_TextSelectedBg] = k_accent_soft;
	c[ImGuiCol_NavCursor] = k_accent;
}

void ImguiManager::show_render_parameters(RenderInfo* info){

	if (!m_show_render) return;

	if (ImGui::Begin("Render", &m_show_render)) {

		if (section("Output")) {
			if (begin_props("##output")) {
				prop_slider("Exposure", &info->exposure, 0.1f, 5.0f);

				prop_label("Tonemapping");
				const char* tonemappers[] = { "Exponential", "ACES" };
				int tonemapper_index = static_cast<int>(info->tonemapper);
				if (ImGui::Combo("##tonemapper", &tonemapper_index, tonemappers, IM_ARRAYSIZE(tonemappers))) {
					info->tonemapper = static_cast<Tonemapper>(tonemapper_index);
				}

				prop_slider("Ambient", &info->ambient_intensity, 0.0f, 3.0f);
				prop_check("Bloom", &info->bloom_active);
				prop_check("FXAA", &info->fxaa_active);
				prop_check("Frustum culling", &info->frustum_culling);

				prop_label("Draw mode");
				int draw_index = static_cast<int>(info->draw_mode);
				if (ImGui::Combo("##draw_mode", &draw_index, m_draw_modes.data(), static_cast<int>(m_draw_modes.size()))) {
					info->draw_mode = static_cast<DrawMode>(draw_index);
				}
				ImGui::EndTable();
			}
		}

		if (section("SSAO")) {
			if (begin_props("##ssao")) {
				prop_check("Enabled", &info->ssao_active);

				ImGui::BeginDisabled(!info->ssao_active);
				prop_check("Half resolution", &info->ssao_half_res);
				ImGui::SetItemTooltip("Computes SSAO at half width and height: about 4 times cheaper, slightly softer.");
				prop_label("Samples");
				ImGui::SliderInt("##samples", &info->ssao_samples, 1, ssao_kernel);
				prop_slider("Radius", &info->ssao_base_radius, 0.1f, 5.0f);
				prop_slider("Max radius", &info->ssao_max_radius_px, 32.0f, 512.0f, "%.0f px");
				ImGui::SetItemTooltip("Screen-space limit. Higher reaches further but costs more up close.");
				prop_slider("Intensity", &info->ssao_intensity, 0.1f, 4.0f);
				prop_slider("Direct", &info->ssao_direct, 0.0f, 1.0f);
				ImGui::SetItemTooltip("How much SSAO also darkens direct light. Ambient and reflections always get it in full.");
				ImGui::EndDisabled();

				ImGui::EndTable();
			}
		}
	}
	ImGui::End();
}

void ImguiManager::init(HWND handle){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();


	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	const char* font_path = "deps/external/imgui/misc/fonts/Roboto-Medium.ttf";
	if (std::filesystem::exists(font_path)) {
		io.Fonts->AddFontFromFileTTF(font_path, 15.0f);
	}

	ImGui_ImplWin32_Init(handle);
	ImGui_ImplDX11_Init(Engine::get_instance()->get_engine_props()->deviceInterface, Engine::get_instance()->get_engine_props()->inmediateDeviceContext);

	apply_nova_style();

	m_current_operation = ImGuizmo::TRANSLATE;
	m_window_handle = handle;
}

void ImguiManager::update_picking(){

	const ImGuiIO& io = ImGui::GetIO();
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) || m_gizmo_hovered || is_mouse_over_ui()) return;

	// Con viewports la posicion del raton es de pantalla; la textura va en pixeles de la ventana
	const ImVec2 mouse = ImGui::GetMousePos();
	const int x = static_cast<int>(mouse.x - m_viewport->Pos.x);
	const int y = static_cast<int>(mouse.y - m_viewport->Pos.y);
	if (x < 0 || y < 0 || x >= static_cast<int>(m_viewport->Size.x) || y >= static_cast<int>(m_viewport->Size.y)) return;

	m_pick_x = x;
	m_pick_y = y;
	m_pick_requested = true;
}

bool ImguiManager::consume_pick_request(int& x, int& y){

	if (!m_pick_requested) return false;

	m_pick_requested = false;
	x = m_pick_x;
	y = m_pick_y;
	return true;
}

void ImguiManager::render_guizmo(CameraComponent* cam/*, ImVec2 pos, ImVec2 size*/) {

	m_gizmo_hovered = false;
	m_camera = cam;

	Entity e = Engine::get_instance()->m_current_scene->get_selected_entity();
	if (e.get_id() != -1) {

		ImGuiIO& io = ImGui::GetIO();

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);

		// Detras de los paneles, y sin robarles el raton
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
		// No se usa WantCaptureMouse: ImGuizmo lo activa al pasar por encima, y desactivarlo por eso
		// haria que se apagase y encendiese en frames alternos.
		ImGuizmo::Enable(ImGuizmo::IsUsing() || !is_mouse_over_ui());

		const ImGuiViewport* HUD_viewport_ = ImGui::GetMainViewport();
		ImGuizmo::SetRect(HUD_viewport_->Pos.x, HUD_viewport_->Pos.y, HUD_viewport_->Size.x, HUD_viewport_->Size.y);


		float* view_matrix = cam->get_view_raw();
		float* projection_matrix = cam->get_projection_raw(); // tu matriz de proyeccion

		// Matriz del objeto a manipular
		float* object_matrix = nullptr;
		TransformComponent* t = Engine::get_instance()->m_current_scene->m_ecs.get_component<TransformComponent>(e);
		if (t) {
			object_matrix = t->get_transform_raw();
		}

		// Una luz direccional no tiene posicion: no hay nada que manipular
		if (object_matrix == nullptr) return;

		ImGuizmo::Manipulate(
			view_matrix,
			projection_matrix,
			m_current_operation,
			ImGuizmo::WORLD,
			object_matrix
		);

		m_gizmo_hovered = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

		// For physics in future (picking)
		//if(ImGuizmo::IsOver())

		if (ImGuizmo::IsUsing()) {

			if (t) {
				const DirectX::XMFLOAT4X4 world(object_matrix);
				t->set_world_matrix(DirectX::XMLoadFloat4x4(&world));
			}

		}
	}
}

void ImguiManager::build_default_layout(unsigned int dockspace_id){

	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoCloseButton);
	ImGui::DockBuilderSetNodeSize(dockspace_id, m_viewport->Size);

	ImGuiID center = dockspace_id;
	ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.16f, nullptr, &center);
	ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
	ImGuiID right_bottom = 0;
	ImGuiID right_top = ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.5f, nullptr, &right_bottom);

	ImGui::DockBuilderDockWindow("Hierarchy", left);
	ImGui::DockBuilderDockWindow("Inspector", right_top);
	ImGui::DockBuilderDockWindow("Camera", right_top);
	ImGui::DockBuilderDockWindow("Stats", right_bottom);
	ImGui::DockBuilderDockWindow("Render", right_bottom);
	ImGui::DockBuilderDockWindow("G-Buffer", right_bottom);
	ImGui::DockBuilderFinish(dockspace_id);
}

void ImguiManager::process_pending_scene(){

	if (m_pending_scene.empty() || Engine::get_instance()->m_resource.is_loading_models()) return;

	const std::string filename = m_pending_scene;
	m_pending_scene.clear();
	open_scene(filename);
}

void ImguiManager::open_scene(const std::string& filename){

	printf("Opening scene: %s\n", filename.c_str());
	Engine::get_instance()->m_resource.release_non_default();

	Scene* s = Engine::get_instance()->create_scene(filename);
	Engine::get_instance()->set_scene(s);

	auto directional_lights = s->m_ecs.viewComponents<DirectionalLight>();
	if (directional_lights.begin() == directional_lights.end()) {
		Entity directional_light = s->m_ecs.create_entity("Directional Light");
		auto& light = s->m_ecs.add_component<DirectionalLight>(directional_light);
		light.set_color({ 1.0f, 1.0f, 1.0f });
		light.set_direction({ 0.5f,-1.0f, -1.0f });
		light.set_enabled(true);
		s->m_ecs.add_component<TransformComponent>(directional_light);
		s->reset_history();
	}
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

	// La disposicion por defecto solo se construye si imgui.ini no tiene una guardada
	ImGuiID dockspace_id = ImGui::GetID("NovaDockSpace");
	if (m_reset_layout || ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
		build_default_layout(dockspace_id);
		m_reset_layout = false;
	}
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoCloseButton);


	// Menu buttons
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::BeginMenu("Open Scene")) {
				const std::string path = "data/scenes";

				for (const auto& entry : std::filesystem::directory_iterator(path)) {
					if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
						std::string filename = entry.path().filename().string();

						if (ImGui::MenuItem(filename.c_str())) {
							request_open_scene(filename);
						}
					}
				}
				ImGui::EndMenu();
			}

			Scene* current_scene = Engine::get_instance()->m_current_scene;
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				current_scene->save();
			}
			if (ImGui::MenuItem("Save As...")) {
				m_open_save_as = true;
				snprintf(m_save_as_name, sizeof(m_save_as_name), "%s", std::filesystem::path(current_scene->get_path()).filename().string().c_str());
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit")) {
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			Scene* current_scene = Engine::get_instance()->m_current_scene;
			Entity selected = current_scene->get_selected_entity();
			const bool has_selection = selected.get_id() != k_no_entity;

			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, current_scene->can_undo())) current_scene->undo();
			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, current_scene->can_redo())) current_scene->redo();
			ImGui::Separator();
			if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, has_selection)) current_scene->duplicate_entity(selected.get_id());
			if (ImGui::MenuItem("Delete", "Del", false, has_selection)) current_scene->delete_entity(selected.get_id());
			ImGui::Separator();
			if (ImGui::BeginMenu("Create")) {
				create_menu_items(current_scene);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Hierarchy", nullptr, &m_show_hierarchy);
			ImGui::MenuItem("Inspector", nullptr, &m_show_inspector);
			ImGui::MenuItem("Camera", nullptr, &m_show_camera);
			ImGui::MenuItem("Render", nullptr, &m_show_render);
			ImGui::MenuItem("Stats", nullptr, &m_show_stats);
			ImGui::MenuItem("G-Buffer", nullptr, &m_show_gbuffer);
			ImGui::Separator();
			if (ImGui::MenuItem("Reset Layout")) {
				m_show_hierarchy = m_show_inspector = m_show_camera = true;
				m_show_render = m_show_stats = m_show_gbuffer = true;
				m_reset_layout = true;
			}
			ImGui::EndMenu();
		}

		// Herramienta del gizmo
		ImGui::Dummy(ImVec2(12.0f, 0.0f));
		auto gizmo_button = [&](const char* label, ImGuizmo::OPERATION operation, const char* tooltip) {
			const bool selected = m_current_operation == operation;
			if (selected) {
				ImGui::PushStyleColor(ImGuiCol_Button, k_accent);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_accent_hover);
			}
			if (ImGui::SmallButton(label)) {
				m_current_operation = operation;
			}
			if (selected) ImGui::PopStyleColor(2);
			ImGui::SetItemTooltip("%s", tooltip);
		};
		gizmo_button("Move", ImGuizmo::TRANSLATE, "Move gizmo (W)");
		gizmo_button("Scale", ImGuizmo::SCALE, "Scale gizmo (E)");
		gizmo_button("Rotate", ImGuizmo::ROTATE, "Rotate gizmo (R)");

		// FPS a la derecha
		char fps_text[32];
		snprintf(fps_text, sizeof(fps_text), "%.0f FPS", m_last_fps);
		const float fps_width = ImGui::CalcTextSize(fps_text).x;
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - fps_width - 14.0f);
		ImGui::TextColored(fps_color(m_last_fps), "%s", fps_text);

		ImGui::EndMenuBar();
	}
	ImGui::PopStyleVar();

	// Guardar como
	if (m_open_save_as) {
		ImGui::OpenPopup("Save Scene As");
		m_open_save_as = false;
	}
	if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextColored(k_text_dim, "data/scenes/");
		ImGui::SameLine(0.0f, 2.0f);
		ImGui::SetNextItemWidth(280.0f);
		const bool confirmed = ImGui::InputText("##save_as", m_save_as_name, sizeof(m_save_as_name), ImGuiInputTextFlags_EnterReturnsTrue);

		std::string file_name = m_save_as_name;
		if (file_name.size() < 5 || file_name.substr(file_name.size() - 5) != ".yaml") file_name += ".yaml";
		const bool valid = file_name != ".yaml" && file_name.find_first_of("/\\:") == std::string::npos;

		ImGui::BeginDisabled(!valid);
		if (ImGui::Button("Save") || (confirmed && valid)) {
			Engine::get_instance()->m_current_scene->save("data/scenes/" + file_name);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void ImguiManager::system_info(){

	Engine* e = Engine::get_instance();
	const float dt = e->get_delta_time();

	// Grafica por frame; el numero de FPS se promedia para que se pueda leer
	m_dt_history[m_dt_history_index] = dt * 1000.0f;
	m_dt_history_index = (m_dt_history_index + 1) % m_dt_history_size;

	m_time += dt;
	m_fps_frames++;
	if (m_time >= m_update_fps_time) {
		m_last_fps = static_cast<float>(m_fps_frames) / m_time;
		m_time = 0.0f;
		m_fps_frames = 0;
	}

	if (!m_show_stats) return;

	if (ImGui::Begin("Stats", &m_show_stats)) {

		const float frame_ms = m_last_fps > 0.0f ? 1000.0f / m_last_fps : 0.0f;

		ImGui::SetWindowFontScale(1.6f);
		ImGui::TextColored(fps_color(m_last_fps), "%.0f", m_last_fps);
		ImGui::SetWindowFontScale(1.0f);
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(k_text_dim, "FPS   %.2f ms", frame_ms);

		float peak = 0.0f;
		for (float value : m_dt_history) peak = std::max(peak, value);

		char overlay[32];
		snprintf(overlay, sizeof(overlay), "max %.1f ms", peak);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::PlotLines("##frame_time", m_dt_history.data(), m_dt_history_size, m_dt_history_index, overlay, 0.0f, std::max(33.3f, peak) * 1.1f, ImVec2(0.0f, 64.0f));

		float gpu_total = 0.0f;
		for (float ms : m_gpu_ms) gpu_total += ms;

		char gpu_title[48];
		snprintf(gpu_title, sizeof(gpu_title), "GPU   %.2f ms###gpu", gpu_total);
		if (section(gpu_title)) {
			if (begin_timing_table("##gpu_table")) {
				for (int i = 0; i < GPU_PASS_COUNT; i++) {
					timing_row(GpuProfiler::get_pass_name(i), m_gpu_ms[i], gpu_total);
				}
				ImGui::EndTable();
			}
		}

#ifdef MEASURE_TIME
		if (section("CPU###cpu")) {
			if (begin_timing_table("##cpu_table")) {
				timing_row("Present (waits for GPU)", m_window_end_frame_time, frame_ms);
				timing_row("Draw submit", m_draw_time, frame_ms);
				timing_row("ImGui", m_draw_imgui_time, frame_ms);
				timing_row("Begin frame", m_window_begin_frame_time, frame_ms);
				timing_row("Engine update", m_engine_update_time, frame_ms);
				timing_row("Transforms", m_update_transform_time, frame_ms);
				timing_row("Fly camera", m_fly_cam_time, frame_ms);
				ImGui::EndTable();
			}
		}
#endif

		if (section("Scene")) {
			Scene* scene = e->m_current_scene;
			ImGui::TextColored(k_text_dim, "Draw calls");
			ImGui::SameLine(130.0f);
			ImGui::Text("%u", m_draw_calls);

			ImGui::TextColored(k_text_dim, "Meshes");
			ImGui::SameLine(130.0f);
			ImGui::Text("%u / %u", m_visible_meshes, m_total_meshes);

			// Por pase: el prepass y el G-Buffer dibujan estos mismos triangulos cada uno
			ImGui::TextColored(k_text_dim, "Triangles");
			ImGui::SameLine(130.0f);
			ImGui::Text("%llu / %llu", m_visible_triangles, m_total_triangles);

			ImGui::TextColored(k_text_dim, "Entities");
			ImGui::SameLine(130.0f);
			ImGui::Text("%zu", scene ? scene->m_ecs.get_entities().size() : size_t(0));

			ImGui::TextColored(k_text_dim, "Resolution");
			ImGui::SameLine(130.0f);
			ImGui::Text("%.0f x %.0f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

			ImGui::TextColored(k_text_dim, "Depth prepass");
			ImGui::SameLine(130.0f);
#ifdef DEPTH_PREPASS
			ImGui::TextColored(k_green, "On");
#else
			ImGui::TextColored(k_text_dim, "Off");
#endif
		}

		if (section("Input", false)) {
			const bool left_click = e->m_input.is_key_pressed(Key::Mouse::LBUTTON);
			const bool right_click = e->m_input.is_key_pressed(Key::Mouse::RBUTTON);
			ImGui::TextColored(k_text_dim, "Mouse");
			ImGui::SameLine(130.0f);
			ImGui::Text("%d, %d", e->m_input.get_mouse_x(), e->m_input.get_mouse_y());
			ImGui::TextColored(k_text_dim, "Delta");
			ImGui::SameLine(130.0f);
			ImGui::Text("%.0f, %.0f", m_mouse_delta_x, m_mouse_delta_y);
			ImGui::TextColored(k_text_dim, "Buttons");
			ImGui::SameLine(130.0f);
			ImGui::Text("L %d   R %d", left_click ? 1 : 0, right_click ? 1 : 0);
		}

		// add_resource_loaded se llama desde los hilos de carga
		std::lock_guard<std::mutex> locked{ resources_lock };
		std::string resources_title = "Resources (" + std::to_string(m_resources_loaded.size()) + ")###resources";
		if (section(resources_title.c_str(), false)) {
			if (ImGui::SmallButton("Clear")) {
				m_resources_loaded.clear();
			}
			for (const std::string& s : m_resources_loaded) {
				ImGui::TextColored(k_text_dim, "%s", s.c_str());
			}
		}
	}
	ImGui::End();
}

void ImguiManager::show_transform(TransformComponent* trans){

	if (!section("Transform")) return;
	if (!begin_props("##transform")) return;

	Vec3 pos = trans->get_position();
	Vec3 rot = trans->get_rotation();
	Vec3 scl = trans->get_scale();

	rot.x = radToDeg(rot.x);
	rot.y = radToDeg(rot.y);
	rot.z = radToDeg(rot.z);

	if (prop_vec3("Position", pos, 0.0f, 0.1f)) {
		trans->set_position(pos);
	}
	if (prop_vec3("Rotation", rot, 0.0f, 0.5f, "%.1f")) {
		rot.x = degToRad(rot.x);
		rot.y = degToRad(rot.y);
		rot.z = degToRad(rot.z);
		trans->set_rotation(rot);
	}
	if (prop_vec3("Scale", scl, 1.0f, 0.05f)) {
		trans->set_scale(scl);
	}

	ImGui::EndTable();
}

void ImguiManager::show_light(PointLight* light){

	if (!section("Point Light")) return;

	Vec3 color = light->get_color();
	Vec3 specular_color = light->get_specular_color();
	float color_tmp[3] = { color.x, color.y, color.z };
	float spec_color_tmp[3] = { specular_color.x, specular_color.y, specular_color.z };

	bool enabled = light->get_enabled();
	float intensity = light->get_intensity();
	float distance = light->get_distance();
	float fall_start = light->get_fall_start();
	float constant_att = light->get_constant_attenuation();
	float linear_att = light->get_linear_attenuation();
	float quadratic_att = light->get_quadratic_attenuation();
	float specular_strength = light->get_specular_strenght();
	float specular_shininess = light->get_specular_shininess();
	float attenuation = light->get_attenuation();
	float exposure = light->get_expossure();
	float fall_off = light->get_fall_off();

	if (begin_props("##point")) {
		prop_check("Enabled", &enabled);
		prop_color("Color", color_tmp);
		prop_float("Intensity", &intensity, 0.05f, 0.0f, 100000.0f);
		prop_float("Range", &distance, 0.1f, 0.0f, 20000.0f);
		prop_slider("Fade start", &fall_start, 0.0f, 1.0f);
		float pulse_speed = light->get_pulse_speed();
		float pulse_min = light->get_pulse_min();
		prop_float("Pulse speed", &pulse_speed, 0.05f, 0.0f, 50.0f);
		prop_slider("Pulse min", &pulse_min, 0.0f, 1.0f);
		light->set_pulse(pulse_speed, pulse_min);
		if (pulse_speed != 0.0f) {
			prop_label("Current intensity");
			ImGui::Text("%.2f", light->get_current_intensity());
		}
		ImGui::EndTable();
	}

	if (ImGui::TreeNode("Attenuation")) {
		if (begin_props("##point_att")) {
			prop_float("Constant", &constant_att, 0.001f, 0.0f, 0.0f, "%.4f");
			prop_float("Linear", &linear_att, 0.001f, 0.0f, 0.0f, "%.4f");
			prop_float("Quadratic", &quadratic_att, 0.0001f, 0.0f, 0.0f, "%.4f");
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Forward renderer only")) {
		if (begin_props("##point_forward")) {
			prop_float("Spec. strength", &specular_strength, 0.001f, 0.0f, 1.0f, "%.3f");
			prop_color("Spec. color", spec_color_tmp);
			prop_float("Shininess", &specular_shininess, 0.5f);
			prop_float("Attenuation", &attenuation, 0.001f, 0.0f, 0.0f, "%.3f");
			prop_float("Exposure", &exposure, 0.1f, 0.0f, 200.0f);
			prop_float("Fall off", &fall_off, 0.1f, 0.0f, 10000.0f);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	light->set_enabled(enabled);
	light->set_color({ color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strength);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);
	light->set_attenuation(attenuation);
	light->set_constant_attenuation(constant_att);
	light->set_linear_attenuation(linear_att);
	light->set_quadratic_attenuation(quadratic_att);
	light->set_intensity(intensity);
	light->set_expossure(exposure);
	light->set_distance(distance);
	light->set_fall_off(fall_off);
	light->set_fall_start(fall_start);
}

void ImguiManager::show_light(SpotLight* light){

	if (!section("Spot Light")) return;

	Vec3 direction = light->get_direction();
	Vec3 color = light->get_color();
	Vec3 specular_color = light->get_specular_color();
	float color_tmp[3] = { color.x, color.y, color.z };
	float spec_color_tmp[3] = { specular_color.x, specular_color.y, specular_color.z };

	bool enabled = light->get_enabled();
	float intensity = light->get_intensity();
	float distance = light->get_distance();
	float inner = light->get_inner_circle();
	float outer = light->get_outside_circle();
	float constant_att = light->get_constant_attenuation();
	float linear_att = light->get_linear_attenuation();
	float quadratic_att = light->get_quadratic_attenuation();
	float specular_strength = light->get_specular_strenght();
	float specular_shininess = light->get_specular_shininess();
	float exposure = light->get_expossure();

	if (begin_props("##spot")) {
		prop_check("Enabled", &enabled);
		prop_vec3("Direction", direction, 0.0f, 0.01f);
		prop_color("Color", color_tmp);
		prop_float("Intensity", &intensity, 0.05f, 0.0f, 100000.0f);
		prop_float("Range", &distance, 0.1f, 0.0f, 2000.0f);
		prop_slider("Inner angle", &inner, 0.0f, 179.0f, "%.1f deg");
		prop_slider("Outer angle", &outer, 0.0f, 179.0f, "%.1f deg");
		ImGui::EndTable();
	}

	if (ImGui::TreeNode("Attenuation")) {
		if (begin_props("##spot_att")) {
			prop_float("Constant", &constant_att, 0.001f, 0.0f, 0.0f, "%.4f");
			prop_float("Linear", &linear_att, 0.001f, 0.0f, 0.0f, "%.4f");
			prop_float("Quadratic", &quadratic_att, 0.0001f, 0.0f, 0.0f, "%.4f");
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Forward renderer only")) {
		if (begin_props("##spot_forward")) {
			prop_float("Spec. strength", &specular_strength, 0.001f, 0.0f, 1.0f, "%.3f");
			prop_color("Spec. color", spec_color_tmp);
			prop_float("Shininess", &specular_shininess, 0.5f);
			prop_float("Exposure", &exposure, 0.1f, 0.0f, 200.0f);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	light->set_direction(direction);
	light->set_inner_circle(inner);
	light->set_outer_circle(outer);
	light->set_enabled(enabled);
	light->set_color({ color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strength);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);
	light->set_constant_attenuation(constant_att);
	light->set_linear_attenuation(linear_att);
	light->set_quadratic_attenuation(quadratic_att);
	light->set_expossure(exposure);
	light->set_distance(distance);
	light->set_intensity(intensity);
}

void ImguiManager::show_light(DirectionalLight* light){

	if (!section("Directional Light")) return;

	Vec3 direction = light->get_direction();
	Vec3 color = light->get_color();
	Vec3 specular_color = light->get_specular_color();
	float color_tmp[3] = { color.x, color.y, color.z };
	float spec_color_tmp[3] = { specular_color.x, specular_color.y, specular_color.z };

	bool enabled = light->get_enabled();
	float intensity = light->get_intensity();
	float specular_strength = light->get_specular_strenght();
	float specular_shininess = light->get_specular_shininess();

	if (begin_props("##directional")) {
		prop_check("Enabled", &enabled);
		prop_vec3("Direction", direction, 0.0f, 0.005f);
		prop_color("Color", color_tmp);
		prop_float("Intensity", &intensity, 0.01f, 0.0f, 100000.0f);
		ImGui::EndTable();
	}

	if (ImGui::TreeNode("Forward renderer only")) {
		if (begin_props("##directional_forward")) {
			prop_float("Spec. strength", &specular_strength, 0.001f, 0.0f, 1.0f, "%.3f");
			prop_color("Spec. color", spec_color_tmp);
			prop_float("Shininess", &specular_shininess, 0.5f);
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}

	light->set_direction(direction);
	light->set_enabled(enabled);
	light->set_color({ color_tmp[0], color_tmp[1], color_tmp[2] });
	light->set_specular_strenght(specular_strength);
	light->set_specular_color({ spec_color_tmp[0], spec_color_tmp[1], spec_color_tmp[2] });
	light->set_specular_shininess(specular_shininess);
	light->set_intensity(intensity);
}

void ImguiManager::show_mesh_comp(MeshComponent* mesh_comp){

	if (!section("Material")) return;

	auto& meshes = mesh_comp->get_model()->meshes;
	if (meshes.empty()) {
		ImGui::TextColored(k_text_dim, "Loading mesh...");
		return;
	}

	// Un bloque por cada material del archivo; todas sus mallas se editan a la vez
	unsigned int slots = 0;
	for (const Mesh& m : meshes) {
		slots = std::max(slots, m.material_slot + 1);
	}

	bool any_change = false;
	for (unsigned int slot = 0; slot < slots; slot++) {

		Mesh* first = nullptr;
		for (Mesh& m : meshes) {
			if (m.material_slot == slot) {
				first = &m;
				break;
			}
		}
		if (!first) continue;

		ImGui::PushID(static_cast<int>(slot));

		Material& material = first->material;
		Vec4 color_v = material.get_color_value();
		Vec3 color_emissive = material.get_emissive_value();
		Vec2 uv_scale = material.get_uv_scale();
		float color[4] = { color_v.x, color_v.y, color_v.z, color_v.w };
		float emissive[3] = { color_emissive.x, color_emissive.y, color_emissive.z };
		float uv[2] = { uv_scale.x, uv_scale.y };
		float emissive_intensity = material.get_emissive_intensity();
		float emissive_bloom = material.get_emissive_bloom();
		float pulse_speed = material.get_emissive_pulse_speed();
		float pulse_min = material.get_emissive_pulse_min();
		float metallic = material.get_metallic_value();
		float roughness = material.get_roughness_value();

		if (slots > 1) {
			ImGui::SeparatorText(("Material " + std::to_string(slot)).c_str());
		}

		bool changed = false;
		if (begin_props("##material")) {
			changed |= prop_color("Albedo", color);
			changed |= prop_slider("Metallic", &metallic, 0.0f, 1.0f);
			changed |= prop_slider("Roughness", &roughness, 0.0f, 1.0f);
			changed |= prop_color("Emissive", emissive);
			changed |= prop_float("Emissive int.", &emissive_intensity, 0.1f, 0.0f, 100000.0f);
			changed |= prop_slider("Emissive bloom", &emissive_bloom, 0.0f, 1.0f);
			changed |= prop_float("Pulse speed", &pulse_speed, 0.05f, 0.0f, 50.0f);
			changed |= prop_slider("Pulse min", &pulse_min, 0.0f, 1.0f);
			if (pulse_speed != 0.0f) {
				prop_label("Current intensity");
				ImGui::Text("%.2f", material.get_current_emissive_intensity());
			}
			prop_label("UV scale");
			changed |= ImGui::DragFloat2("##uv_scale", uv, 0.05f, 0.01f, 1000.0f);
			ImGui::EndTable();
		}

		if (changed) {
			any_change = true;
			for (Mesh& m : meshes) {
				if (m.material_slot != slot) continue;
				m.material.set_color_value({ color[0], color[1], color[2], color[3] });
				m.material.set_emissive_value({ emissive[0], emissive[1], emissive[2] });
				m.material.set_emissive_intensity(emissive_intensity);
				m.material.set_emissive_bloom(emissive_bloom);
				m.material.set_emissive_pulse(pulse_speed, pulse_min);
				m.material.set_metallic_value(metallic);
				m.material.set_roughness_value(roughness);
				m.material.set_uv_scale({ uv[0], uv[1] });
			}
		}

		ImGui::PopID();
	}

	if (any_change) {
		mesh_comp->store_materials_from_model();
	}
}

void ImguiManager::show_cam(CameraComponent* cam, int entity_id){

	if (!m_show_camera) return;

	if (ImGui::Begin("Camera", &m_show_camera)) {

		Vec3 position = cam->get_position();
		float near_t = cam->get_near();
		float far_t = cam->get_far();
		float pitch = cam->get_pitch();
		float yaw = cam->get_yaw();
		float mov_speed = cam->get_movement_speed();
		float sensitivity = cam->get_mouse_sensitivity();

		if (begin_props("##camera")) {
			prop_vec3("Position", position, 0.0f, 0.1f);
			prop_float("Pitch", &pitch, 0.5f, -89.0f, 89.0f, "%.1f deg");
			prop_float("Yaw", &yaw, 0.5f, 0.0f, 0.0f, "%.1f deg");
			prop_float("Near", &near_t, 0.001f, 0.001f, 1.0f, "%.3f");
			prop_float("Far", &far_t, 1.0f, 1.0f, 100000.0f, "%.0f");
			prop_float("Move speed", &mov_speed, 0.1f, 0.0f, 1000.0f);
			prop_float("Sensitivity", &sensitivity, 0.01f, 0.0f, 5.0f);
			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextColored(k_text_dim, "Hold right mouse to look. WASD moves, the wheel changes speed.");
		ImGui::PopTextWrapPos();

		cam->set_position(position);
		cam->set_near(near_t);
		cam->set_far(far_t);
		cam->set_pitch(pitch);
		cam->set_yaw(yaw);
		cam->set_movement_speed(mov_speed);
		cam->set_mouse_sensitivity(sensitivity);
	}
	ImGui::End();
}

void ImguiManager::add_resource_loaded(std::string text){

	std::lock_guard<std::mutex> locked{ resources_lock };
	m_resources_loaded.push_back(text);
}

void ImguiManager::create_menu_items(Scene* scene){

	// Delante de la camara
	Vec3 position = { 0.0f, 0.0f, 0.0f };
	if (m_camera) {
		const Vec3 cam = m_camera->get_position();
		const float yaw = degToRad(m_camera->get_yaw());
		const float pitch = degToRad(m_camera->get_pitch());
		const float distance = 8.0f;
		position = {
			cam.x + cosf(yaw) * cosf(pitch) * distance,
			cam.y + sinf(pitch) * distance,
			cam.z + sinf(-yaw) * cosf(pitch) * distance,
		};
	}

	if (ImGui::MenuItem("Empty")) scene->create_entity_from_template(EntityTemplate::Empty, position);
	ImGui::Separator();
	if (ImGui::MenuItem("Cube")) scene->create_entity_from_template(EntityTemplate::Cube, position);
	if (ImGui::MenuItem("Sphere")) scene->create_entity_from_template(EntityTemplate::Sphere, position);
	if (ImGui::MenuItem("Cylinder")) scene->create_entity_from_template(EntityTemplate::Cylinder, position);
	if (ImGui::MenuItem("Cone")) scene->create_entity_from_template(EntityTemplate::Cone, position);
	ImGui::Separator();
	if (ImGui::MenuItem("Point Light")) scene->create_entity_from_template(EntityTemplate::PointLight, position);
	if (ImGui::MenuItem("Spot Light")) scene->create_entity_from_template(EntityTemplate::SpotLight, position);
	if (ImGui::MenuItem("Directional Light")) scene->create_entity_from_template(EntityTemplate::DirectionalLight, position);
}

void ImguiManager::handle_shortcuts(Scene* scene){

	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput) return;

	Entity selected = scene->get_selected_entity();
	const bool has_selection = selected.get_id() != k_no_entity;

	if (io.KeyCtrl) {
		if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
			if (io.KeyShift) scene->redo();
			else scene->undo();
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) scene->redo();
		if (ImGui::IsKeyPressed(ImGuiKey_S, false)) scene->save();
		if (has_selection && ImGui::IsKeyPressed(ImGuiKey_D, false)) scene->duplicate_entity(selected.get_id());
	}
	else if (has_selection && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
		scene->delete_entity(selected.get_id());
	}
}

void ImguiManager::show_hierarchy(Scene* scene){

	if (!m_show_hierarchy) return;

	if (ImGui::Begin("Hierarchy", &m_show_hierarchy)) {

		EntityComponentSystem& ecs = scene->m_ecs;
		std::vector<Entity>& entities = ecs.get_entities();
		const unsigned int selected_id = scene->get_selected_entity().get_id();

		if (ImGui::Button("+")) {
			ImGui::OpenPopup("##create_entity");
		}
		ImGui::SetItemTooltip("Create entity");
		if (ImGui::BeginPopup("##create_entity")) {
			create_menu_items(scene);
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::InputTextWithHint("##search", "Search...", m_filter.InputBuf, IM_ARRAYSIZE(m_filter.InputBuf))) {
			m_filter.Build();
		}

		const bool filtering = m_filter.IsActive();

		// Arbol: se reconstruye cada frame a partir del padre de cada transform
		struct Node {
			Entity* entity;
			TransformComponent* transform;
			std::vector<int> children;
		};
		std::vector<Node> nodes;
		nodes.reserve(entities.size());
		std::unordered_map<const TransformComponent*, int> node_by_transform;
		std::unordered_map<unsigned int, int> node_by_id;
		for (Entity& e : entities) {
			TransformComponent* t = ecs.get_component<TransformComponent>(e);
			const int index = static_cast<int>(nodes.size());
			nodes.push_back({ &e, t, {} });
			node_by_id[e.get_id()] = index;
			if (t) node_by_transform[t] = index;
		}

		std::vector<int> roots;
		for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
			TransformComponent* parent = nodes[i].transform ? nodes[i].transform->get_parent() : nullptr;
			auto it = parent ? node_by_transform.find(parent) : node_by_transform.end();
			if (it != node_by_transform.end() && !filtering) nodes[it->second].children.push_back(i);
			else roots.push_back(i);
		}

		// El cambio de padre se aplica al acabar de dibujar
		enum class DropMode { Into, Before, After };
		int reparent_child = -1;
		int reparent_target = -1;
		DropMode drop_mode = DropMode::Into;
		const char* payload_type = "NOVA_ENTITY";

		ImGui::BeginChild("##entities");
		ImDrawList* draw = ImGui::GetWindowDrawList();

		std::function<void(int)> draw_node = [&](int index) {
			Node& node = nodes[index];
			Entity& e = *node.entity;
			const std::string name = e.get_name();

			if (filtering && !m_filter.PassFilter(name.c_str())) return;

			DirectionalLight* directional = ecs.get_component<DirectionalLight>(e);
			PointLight* point = ecs.get_component<PointLight>(e);
			SpotLight* spot = ecs.get_component<SpotLight>(e);
			MeshComponent* mesh = ecs.get_component<MeshComponent>(e);

			const char* type = "Entity";
			bool is_light = false;
			bool enabled = true;
			Vec3 light_color = { 1.0f, 1.0f, 1.0f };
			if (directional) { type = "Directional"; is_light = true; enabled = directional->get_enabled(); light_color = directional->get_color(); }
			else if (point) { type = "Point"; is_light = true; enabled = point->get_enabled(); light_color = point->get_color(); }
			else if (spot) { type = "Spot"; is_light = true; enabled = spot->get_enabled(); light_color = spot->get_color(); }
			else if (mesh) { type = "Mesh"; }

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
			if (node.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
			if (e.get_id() == selected_id) flags |= ImGuiTreeNodeFlags_Selected;

			const float row_x = ImGui::GetCursorScreenPos().x;
			const float label_spacing = ImGui::GetTreeNodeToLabelSpacing();

			// Por nombre: el arbol conserva lo plegado aunque el historial reconstruya la escena
			ImGui::PushID(name.c_str());
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
			const bool open = ImGui::TreeNodeEx("##row", flags);
			ImGui::PopStyleVar();

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
				scene->select_entity(e);
			}

			if (ImGui::BeginPopupContextItem("##row_menu")) {
				if (ImGui::MenuItem("Duplicate", "Ctrl+D")) scene->duplicate_entity(e.get_id());
				if (ImGui::MenuItem("Delete", "Del")) scene->delete_entity(e.get_id());
				ImGui::EndPopup();
			}

			const ImVec2 min = ImGui::GetItemRectMin();
			const ImVec2 max = ImGui::GetItemRectMax();

			// Solo lo que tiene transform puede tener padre o hijos
			if (node.transform) {
				if (ImGui::BeginDragDropSource()) {
					const unsigned int id = e.get_id();
					ImGui::SetDragDropPayload(payload_type, &id, sizeof(id));
					ImGui::TextUnformatted(name.c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					const ImGuiPayload* peek = ImGui::GetDragDropPayload();
					if (peek && peek->IsDataType(payload_type)) {
						auto it = node_by_id.find(*static_cast<const unsigned int*>(peek->Data));
						TransformComponent* dragged = it != node_by_id.end() ? nodes[it->second].transform : nullptr;

						// Tercio superior: encima. Inferior: debajo. Centro: hija.
						const float relative = (ImGui::GetMousePos().y - min.y) / std::max(max.y - min.y, 1.0f);
						const DropMode mode = relative < 0.25f ? DropMode::Before : (relative > 0.75f ? DropMode::After : DropMode::Into);

						const bool valid = dragged && dragged != node.transform && !node.transform->is_descendant_of(dragged);
						if (valid) {
							if (mode == DropMode::Into) {
								draw->AddRect(min, max, ImGui::GetColorU32(k_accent), 3.0f, 0, 1.5f);
							}
							else {
								const float y = mode == DropMode::Before ? min.y : max.y;
								draw->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), ImGui::GetColorU32(k_accent), 2.0f);
							}

							if (ImGui::AcceptDragDropPayload(payload_type, ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
								reparent_child = it->second;
								reparent_target = index;
								drop_mode = mode;
							}
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			if (ImGui::IsItemVisible()) {
				const float center_y = (min.y + max.y) * 0.5f;
				const float text_y = center_y - ImGui::GetTextLineHeight() * 0.5f;
				const ImVec2 icon(row_x + label_spacing + 5.0f, center_y);

				if (is_light) {
					const ImU32 color = ImGui::GetColorU32(ImVec4(light_color.x, light_color.y, light_color.z, 1.0f));
					if (enabled) draw->AddCircleFilled(icon, 4.5f, color);
					else draw->AddCircle(icon, 4.5f, ImGui::GetColorU32(k_text_dim), 0, 1.5f);
				}
				else {
					draw->AddRectFilled(ImVec2(icon.x - 4.0f, icon.y - 4.0f), ImVec2(icon.x + 4.0f, icon.y + 4.0f), ImGui::GetColorU32(k_text_dim), 1.5f);
				}

				const float type_width = ImGui::CalcTextSize(type).x;
				const float type_x = max.x - type_width - 6.0f;
				const float name_x = icon.x + 12.0f;
				const bool show_type = type_x - name_x > ImGui::CalcTextSize(name.c_str()).x + 8.0f;

				draw->PushClipRect(min, max, true);
				draw->AddText(ImVec2(name_x, text_y), ImGui::GetColorU32(enabled ? k_text : k_text_dim), name.c_str());
				if (show_type) draw->AddText(ImVec2(type_x, text_y), ImGui::GetColorU32(k_text_dim), type);
				draw->PopClipRect();
			}

			if (open) {
				for (int child : node.children) draw_node(child);
				ImGui::TreePop();
				// ImGui 1.92 WIP no limpia esto en TreePop: si un padre posterior queda fuera de la vista, sus hijos
				// creen tener datos en TreeNodeStack y escriben antes del inicio del buffer, corrompiendo el heap.
				ImGuiWindow* tree_window = ImGui::GetCurrentWindow();
				tree_window->DC.TreeRecordsClippedNodesY2Mask &= ~(1 << tree_window->DC.TreeDepth);
			}
			ImGui::PopID();
		};

		for (int index : roots) draw_node(index);

		// Soltar fuera de cualquier fila: la entidad pasa a la raiz
		const bool dragging_entity = ImGui::GetDragDropPayload() && ImGui::GetDragDropPayload()->IsDataType(payload_type);
		if (dragging_entity) {
			ImGui::Spacing();
			ImGui::TextColored(k_text_dim, "Drop here to remove the parent");
		}
		ImGuiWindow* entities_window = ImGui::GetCurrentWindow();
		if (ImGui::BeginDragDropTargetCustom(entities_window->InnerRect, entities_window->ID)) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_type)) {
				auto it = node_by_id.find(*static_cast<const unsigned int*>(payload->Data));
				if (it != node_by_id.end()) {
					reparent_child = it->second;
					reparent_target = -1;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (reparent_child != -1 && nodes[reparent_child].transform) {
			TransformComponent* child = nodes[reparent_child].transform;

			TransformComponent* new_parent = nullptr;
			if (reparent_target != -1) {
				new_parent = drop_mode == DropMode::Into ? nodes[reparent_target].transform : nodes[reparent_target].transform->get_parent();
			}
			if (child->get_parent() != new_parent) {
				child->set_parent_keep_world(new_parent);
			}

			if (reparent_target != -1 && drop_mode != DropMode::Into) {
				const unsigned int child_id = nodes[reparent_child].entity->get_id();
				const unsigned int target_id = nodes[reparent_target].entity->get_id();
				ecs.move_entity(child_id, target_id, drop_mode == DropMode::After);
			}
			scene->request_history_check();
		}

		// Clic en vacio: quita la seleccion
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			scene->select_entity(Entity(k_no_entity, ""));
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void ImguiManager::show_inspector(Scene* scene){

	if (!m_show_inspector) return;

	if (ImGui::Begin("Inspector", &m_show_inspector)) {

		Entity e = scene->get_selected_entity();
		EntityComponentSystem& ecs = scene->m_ecs;

		if (e.get_id() == k_no_entity) {
			centered_hint("Select an entity in the Hierarchy");
		}
		else {
			const std::string name = e.get_name();
			if (m_rename_entity_id != e.get_id() || !ImGui::IsAnyItemActive()) {
				m_rename_entity_id = e.get_id();
				snprintf(m_rename_buffer, sizeof(m_rename_buffer), "%s", name.c_str());
			}
			ImGui::SetNextItemWidth(-60.0f);
			ImGui::InputText("##entity_name", m_rename_buffer, sizeof(m_rename_buffer));
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				scene->rename_entity(e.get_id(), m_rename_buffer);
			}
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(k_text_dim, "ID %u", e.get_id());

			TransformComponent* selected_transform = ecs.get_component<TransformComponent>(e);
			TransformComponent* parent = selected_transform ? selected_transform->get_parent() : nullptr;
			if (parent) {
				std::string parent_name = "?";
				for (Entity& other : ecs.get_entities()) {
					if (ecs.get_component<TransformComponent>(other) == parent) {
						parent_name = other.get_name();
						break;
					}
				}
				ImGui::TextColored(k_text_dim, "Parent: %s", parent_name.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Unparent")) {
					selected_transform->set_parent_keep_world(nullptr);
				}
			}
			ImGui::Spacing();

			if (TransformComponent* trans = ecs.get_component<TransformComponent>(e)) show_transform(trans);
			if (DirectionalLight* light = ecs.get_component<DirectionalLight>(e)) show_light(light);
			if (PointLight* light = ecs.get_component<PointLight>(e)) show_light(light);
			if (SpotLight* light = ecs.get_component<SpotLight>(e)) show_light(light);
			if (MeshComponent* mesh = ecs.get_component<MeshComponent>(e)) show_mesh_comp(mesh);
		}
	}
	ImGui::End();
}

void ImguiManager::scene_info(Scene* scene){
	show_hierarchy(scene);
	show_inspector(scene);
	handle_shortcuts(scene);

	// Al acabar una edicion (soltar un control o el gizmo), el historial mira si la escena ha cambiado
	const bool interacting = ImGui::IsAnyItemActive() || ImGuizmo::IsUsing();
	if (m_was_interacting && !interacting) {
		scene->request_history_check();
	}
	m_was_interacting = interacting;
}

void ImguiManager::gbuffer_info(DeferredResources* gbuffer){

	if (!m_show_gbuffer) return;

	if (ImGui::Begin("G-Buffer", &m_show_gbuffer)) {

		struct Slot { const char* name; ID3D11ShaderResourceView* srv; };
		const Slot slots[] = {
			{ "Albedo", gbuffer->gbuffer_albedo_shader_resource_view },
			{ "Normals", gbuffer->gbuffer_normals_shader_resource_view },
			{ "Material", gbuffer->gbuffer_material_shader_resource_view },
			{ "Emissive", gbuffer->gbuffer_emissive_shader_resource_view },
			{ "Bloom", gbuffer->emissive_dowscaling_shader_resource_view[0] },
			{ "Depth", gbuffer->depth_srv },
			{ "SSAO", gbuffer->ssao_shader_resource_view },
			{ "SSAO blur", gbuffer->ssao_blur_shader_resource_view },
		};

		const ImGuiIO& io = ImGui::GetIO();
		const float aspect = io.DisplaySize.y > 0.0f ? io.DisplaySize.x / io.DisplaySize.y : 1.0f;
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float avail = ImGui::GetContentRegionAvail().x;
		const int columns = std::max(1, static_cast<int>((avail + spacing) / (150.0f + spacing)));
		const float width = (avail - spacing * (columns - 1)) / columns;
		const ImVec2 size(width, width / aspect);

		int column = 0;
		for (const Slot& slot : slots) {
			ImGui::BeginGroup();
			ImGui::TextColored(k_text_dim, "%s", slot.name);
			ImGui::Image((ImTextureID)slot.srv, size);
			ImGui::EndGroup();

			if (ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
				ImGui::TextUnformatted(slot.name);
				ImGui::Image((ImTextureID)slot.srv, ImVec2(480.0f, 480.0f / aspect));
				ImGui::EndTooltip();
			}

			column++;
			if (column < columns) ImGui::SameLine();
			else column = 0;
		}
	}
	ImGui::End();
}
