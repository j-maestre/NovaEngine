#pragma once
#include "memory"

struct WindowInfo;
struct WindowProperties;


class Window {

public:
	Window();
	Window(Window&&) = delete;
	Window(const Window&);
	~Window();

	void init(const WindowProperties* props);
private:

	std::shared_ptr<WindowInfo> m_window_info;
	bool m_initialized;
};
