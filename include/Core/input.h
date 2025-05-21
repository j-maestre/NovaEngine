#pragma once
#include "Core/defines.h"
#include "Core/keys.h"
#include <memory>
#include <vector>
#include <unordered_map>

class Window;

class Input{

private:

public:
	Input();
	Input(Input&&) = delete;
	Input(const Input&) = delete;
	~Input();

	bool is_key_down(Key::Keyboard key) const;
	bool is_key_pressed(Key::Keyboard key) const;
	bool is_key_up(Key::Keyboard key) const;

	bool is_key_down(Key::Mouse key) const;
	bool is_key_pressed(Key::Mouse key) const;
	bool is_key_up(Key::Mouse key) const;

	bool is_mouse_wheel_up() const;
	bool is_mouse_wheel_down() const;

	int get_mouse_x() const;
	int get_mouse_y() const;

private:

	friend class Window;

	std::unordered_map<Key::Keyboard, Key::KeyState> m_keyboard;
	std::unordered_map<Key::Pad, Key::KeyState> m_pad;
	
	std::unordered_map<Key::Mouse, Key::KeyState> m_mouse;
	int m_mouse_x;
	int m_mouse_y;

	bool m_mouse_wheel_up;
	bool m_mouse_wheel_down;

};