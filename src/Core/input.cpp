#include "Core/input.h"
#include <Core/defines.h>
#include <assert.h>

Input::Input(){

	m_mouse_x = 0.0f;
	m_mouse_y = 0.0f;

	m_mouse.emplace(std::pair(Key::Mouse::LBUTTON, Key::KeyState::Release));
	m_mouse.emplace(std::pair(Key::Mouse::RBUTTON, Key::KeyState::Release));

	m_keyboard.emplace(std::pair(Key::Keyboard::BACK, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::TAB, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::CLEAR, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RETURN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SHIFT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::CONTROL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MENU, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PAUSE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::CAPITAL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::KANA, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::IME_ON, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::JUNJA, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::FINAL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::HANJA, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::IME_OFF, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::ESCAPE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::CONVERT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NONCONVERT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::ACCEPT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MODECHANGE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SPACE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PRIOR, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NEXT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::END, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::HOME, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LEFT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::UP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RIGHT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::DOWN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SELECT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PRINT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::EXECUTE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SNAPSHOT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::INSERT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::DELETE_, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::HELP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM0, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM2, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM3, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM4, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM5, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM6, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM7, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM8, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM9, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::A, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::B, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::C, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::D, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::E, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::G, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::H, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::I, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::J, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::K, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::L, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::M, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::N, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::O, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::P, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::Q, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::R, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::S, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::T, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::U, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::V, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::W, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::X, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::Y, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::Z, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LWIN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RWIN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::APPS, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SLEEP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD0, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD2, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD3, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD4, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD5, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD6, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD7, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD8, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD9, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MULTIPLY, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::ADD, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SEPARATOR, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SUBTRACT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::DECIMAL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::DIVIDE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F2, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F3, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F4, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F5, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F6, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F7, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F8, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F9, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F10, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F11, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F12, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F13, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F14, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F15, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F16, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F17, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F18, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F19, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F20, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F21, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F22, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F23, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::F24, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMLOCK, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::SCROLL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LSHIFT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RSHIFT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LCONTROL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RCONTROL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LMENU, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::RMENU, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_BACK, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_FORWARD, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_REFRESH, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_STOP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_SEARCH, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_FAVORITES, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_HOME, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_MUTE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_DOWN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_UP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_NEXT_TRACK, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_PREV_TRACK, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_STOP, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_PLAY_PAUSE, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_MAIL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_MEDIA_SELECT, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_APP1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_APP2, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_PLUS, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_COMMA, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_MINUS, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_PERIOD, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_2, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_3, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_4, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_5, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_6, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_7, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_8, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_102, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PROCESSKEY, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::ATTN, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::CRSEL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::EXSEL, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::EREOF, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PLAY, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::ZOOM, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::PA1, Key::KeyState::Release));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_CLEAR, Key::KeyState::Release));
}

Input::~Input()
{
}


bool Input::is_key_down(Key::Keyboard key) const{
	return m_keyboard.at(key) == Key::KeyState::Down;
}

bool Input::is_key_pressed(Key::Keyboard key) const{

	return m_keyboard.at(key) == Key::KeyState::Pressed;
}

bool Input::is_key_up(Key::Keyboard key) const{
	return m_keyboard.at(key) == Key::KeyState::Release;
}

float Input::get_mouse_x() const
{
	return m_mouse_x;
}

float Input::get_mouse_y() const
{
	return m_mouse_y;
}
