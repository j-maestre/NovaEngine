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
	bool is_key_up(Key::Keyboard key) const;

	float get_mouse_x() const;
	float get_mouse_y() const;

private:

	friend class Window;

	std::unordered_map<Key::Keyboard, Key::KeyState> m_keyboard;
	std::unordered_map<Key::Pad, Key::KeyState> m_pad;
	
	std::unordered_map<Key::Mouse, Key::KeyState> m_mouse;
	float m_mouse_x;
	float m_mouse_y;

};