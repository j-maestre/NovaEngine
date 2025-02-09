#pragma once
#include "memory"

struct WindowInfo;
struct WindowProperties;


class Window {

public:
	Window();
	Window(const Window&) = delete;
	Window(Window&&);
	~Window();

	void init(const WindowProperties* props);
	bool update();
private:

	std::shared_ptr<WindowInfo> m_window_info;
	bool m_initialized;
};
