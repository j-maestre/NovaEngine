#pragma once
#include "memory"
#include "Core/input.h"

struct WindowInfo;
struct WindowProperties;

struct IDXGISwapChain;
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
	
	void set_full_screen();
	void set_windowed();
	void resize();

	friend class Engine;

private:

	friend class Renderer;

	Input* m_input;
	ImguiManager* m_imgui;
	Renderer* m_renderer;

	// Render stuff
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_deviceInterface;
	ID3D11DeviceContext* m_inmediateDeviceContext;

	std::shared_ptr<WindowInfo> m_window_info;
	std::shared_ptr<WindowProperties> m_window_props;
	bool m_initialized;

	float m_width;
	float m_height;
};
