#pragma once
#include "memory"
#include "Core/input.h"

struct WindowInfo;
struct WindowProperties;

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;

class Window {

public:
	Window();
	Window(const Window&) = delete;
	Window(Window&&);
	~Window();

	void init(const WindowProperties* props);
	void begin_frame();
	void end_frame();
	bool update();

	void release();

	WindowInfo* get_window_info();
	WindowProperties* get_window_properties();

	void process_key(WPARAM param);
	void process_mouse(LPARAM param);

	friend class Engine;
private:

	Input m_input;


	// Render stuff
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_deviceInterface;
	ID3D11DeviceContext* m_inmediateDeviceContext;

	std::shared_ptr<WindowInfo> m_window_info;
	std::shared_ptr<WindowProperties> m_window_props;
	bool m_initialized;
};
