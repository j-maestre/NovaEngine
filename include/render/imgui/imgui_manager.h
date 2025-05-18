#pragma once
#define IMGUI_IMPL_WIN32
#define IMGUI_ENABLE_VIEWPORTS
#define IMGUI_HAS_DOCK
#define IMGUI_ENABLE_DOCKING
//#define IMGUI_DEFINE_MATH_OPERATORS

#include "Core/defines.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Core/engine.h"
#include "Core/ECS.h"

#include "components/transform_component.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/Lights/DirectionalLight.h"



class Window;
class Renderer;
class ImguiManager {

private:
	ImguiManager();
public:

	static ImguiManager* get_instance() {
		static ImguiManager instance;
		return &instance;
	}

	ImguiManager(const ImguiManager&);
	ImguiManager(ImguiManager&&);
	~ImguiManager();

	void begin_frame();
	void end_frame();
	void render();

	void resize(HWND hwnd, unsigned int width, unsigned int height);
	void scene_info(EntityComponentSystem& ecs);

	void apply_nova_style();
	void show_demo_window();

	void show_cam(CameraComponent* cam, int entity_id);

private:
	friend class Window;
	friend class CameraComponent;
	friend class Renderer;

	void init(HWND handle);

	void main_menu();
	void system_info();

	void show_transform(TransformComponent* trans, int entity_id);
	void show_directional(DirectionalLight* light, int entity_id);

	float m_time;
	float m_last_fps;
	float m_update_fps_time;

	std::vector<float> m_dt_history;
	int m_dt_history_size;
	int m_dt_history_index;

	float m_mouse_delta_x = 0.0f;
	float m_mouse_delta_y = 0.0f;

	float m_draw_time = 0.0f;

	ImGuiViewport* m_viewport;
};