#pragma once
#define IMGUI_IMPL_WIN32
#define IMGUI_ENABLE_VIEWPORTS
#define IMGUI_HAS_DOCK
#define IMGUI_ENABLE_DOCKING

#include "Core/defines.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Core/engine.h"


class Window;
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

	void show_demo_window();

private:
	friend class Window;
	void init(HWND handle);

	void main_menu();

	ImGuiViewport* m_viewport;
};