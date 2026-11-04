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
#include "Core/ImGuizmo.h"
#include "render/gpu_profiler.h"

#include "Core/engine.h"
#include "Core/ECS.h"
#include "Core/Scene.h"

#include "components/transform_component.h"
#include "components/camera_component.h"
#include "components/mesh_component.h"
#include "components/Lights/DirectionalLight.h"
#include "components/Lights/PointLight.h"
#include "components/Lights/SpotLight.h"



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
	void scene_info(Scene* scene);
	void gbuffer_info(DeferredResources* gbuffer);

	void apply_nova_style();

	void show_render_parameters(RenderInfo* info);

	void show_cam(CameraComponent* cam, int entity_id);
	void render_guizmo(CameraComponent* cam/*, ImVec2 pos, ImVec2 size*/);

	void add_resource_loaded(std::string text);

	std::string m_resources = "";
private:
	friend class Engine;
	friend class Window;
	friend class CameraComponent;
	friend class Renderer;
	friend class Scene;

	void init(HWND handle);


	void main_menu();
	void build_default_layout(unsigned int dockspace_id);
	void open_scene(const std::string& filename);
	void system_info();

	void show_hierarchy(Scene* scene);
	void show_inspector(Scene* scene);
	void show_transform(TransformComponent* trans);
	void show_light(DirectionalLight* light);
	void show_light(PointLight* light);
	void show_light(SpotLight* light);
	void show_mesh_comp(MeshComponent* mesh_comp);

	std::vector<std::string> m_resources_loaded;

	float m_time;
	float m_last_fps;
	float m_update_fps_time;
	int m_fps_frames = 0;

	// Paneles visibles (menu View)
	bool m_show_hierarchy = true;
	bool m_show_inspector = true;
	bool m_show_camera = true;
	bool m_show_render = true;
	bool m_show_stats = true;
	bool m_show_gbuffer = true;
	bool m_reset_layout = false;

	ImGuiTextFilter m_filter;

	std::vector<float> m_dt_history;
	int m_dt_history_size;
	int m_dt_history_index;

	unsigned int m_draw_calls = 0;

	float m_mouse_delta_x = 0.0f;
	float m_mouse_delta_y = 0.0f;

	float m_gpu_ms[GPU_PASS_COUNT] = {};

	float m_draw_time = 0.0f;
	float m_update_transform_time = 0.0f;
	float m_draw_imgui_time = 0.0f;
	float m_update_cam_time = 0.0f;
	float m_fly_cam_time = 0.0f;
	float m_engine_update_time = 0.0f;
	float m_window_begin_frame_time = 0.0f;
	float m_window_end_frame_time = 0.0f;

	ImGuizmo::OPERATION m_current_operation;

	bool m_bloom = true;
	DrawMode m_current_draw_mode;

	std::vector<const char*> m_draw_modes;

	std::mutex resources_lock;
	ImGuiViewport* m_viewport;

	HWND m_window_handle;
};