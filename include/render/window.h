#pragma once
#include "memory"
#include "Core/input.h"


struct WindowInfo;
struct WindowProperties;

struct IDXGISwapChain1;
struct ID3D11Device;
struct ID3D11DeviceContext;
class ImguiManager;
class Renderer;

class Window {

public:
	Window();
	Window(const Window&) = delete;
	Window(Window&&);
	~Window();

	void init(WindowProperties* props);
	void init_imgui();
	void begin_frame();
	void end_frame();
	bool update();

	void release();

	WindowInfo* get_window_info();
	WindowProperties* get_window_properties();

	void process_key(WPARAM param, bool down = true);
	void process_mouse(LPARAM param);
	void process_mouse_button(Key::Mouse btn, Key::KeyState state);
	void process_mouse_wheel(bool up);
	
	void set_full_screen();
	void set_windowed();
	void resize();

	friend class Engine;

private:

	unsigned int m_vsync = 1;

	friend class Renderer;

	void destroy_frame_resources();
	void create_frame_resources();
	__forceinline void create_full_screen_quad_resources(unsigned int width, unsigned int height);

	Input* m_input;
	ImguiManager* m_imgui;
	Renderer* m_renderer;

	// Render stuff
	ComPtr<IDXGISwapChain3> m_swapChain;
	ID3D11Device* m_deviceInterface;
	ID3D11DeviceContext* m_inmediateDeviceContext;

	std::shared_ptr<WindowInfo> m_window_info;
	std::shared_ptr<WindowProperties> m_window_props;
	bool m_initialized;

	float m_width;
	float m_height;
};
