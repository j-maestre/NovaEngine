#include "Core/input.h"
#include <Core/defines.h>
#include <assert.h>

Input::Input(){

	m_mouse_x = 0.0f;
	m_mouse_y = 0.0f;

	m_mouse.emplace(std::pair(Key::Mouse::LBUTTON, Key::KeyState::Up));
	m_mouse.emplace(std::pair(Key::Mouse::RBUTTON, Key::KeyState::Up));

	m_keyboard.emplace(std::pair(Key::Keyboard::BACK, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::TAB, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::CLEAR, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RETURN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SHIFT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::CONTROL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MENU, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PAUSE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::CAPITAL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::KANA, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::IME_ON, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::JUNJA, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::FINAL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::HANJA, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::IME_OFF, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::ESCAPE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::CONVERT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NONCONVERT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::ACCEPT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MODECHANGE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SPACE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PRIOR, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NEXT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::END, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::HOME, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LEFT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::UP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RIGHT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::DOWN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SELECT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PRINT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::EXECUTE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SNAPSHOT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::INSERT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::DELETE_, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::HELP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM0, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM2, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM3, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM4, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM5, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM6, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM7, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM8, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUM9, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::A, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::B, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::C, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::D, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::E, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::G, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::H, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::I, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::J, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::K, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::L, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::M, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::N, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::O, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::P, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::Q, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::R, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::S, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::T, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::U, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::V, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::W, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::X, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::Y, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::Z, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LWIN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RWIN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::APPS, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SLEEP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD0, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD2, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD3, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD4, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD5, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD6, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD7, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD8, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMPAD9, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MULTIPLY, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::ADD, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SEPARATOR, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SUBTRACT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::DECIMAL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::DIVIDE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F2, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F3, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F4, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F5, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F6, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F7, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F8, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F9, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F10, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F11, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F12, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F13, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F14, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F15, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F16, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F17, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F18, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F19, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F20, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F21, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F22, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F23, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::F24, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::NUMLOCK, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::SCROLL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LSHIFT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RSHIFT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LCONTROL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RCONTROL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LMENU, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::RMENU, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_BACK, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_FORWARD, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_REFRESH, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_STOP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_SEARCH, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_FAVORITES, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::BROWSER_HOME, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_MUTE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_DOWN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::VOLUME_UP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_NEXT_TRACK, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_PREV_TRACK, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_STOP, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::MEDIA_PLAY_PAUSE, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_MAIL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_MEDIA_SELECT, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_APP1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::LAUNCH_APP2, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_PLUS, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_COMMA, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_MINUS, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_PERIOD, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_2, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_3, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_4, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_5, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_6, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_7, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_8, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_102, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PROCESSKEY, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::ATTN, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::CRSEL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::EXSEL, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::EREOF, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PLAY, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::ZOOM, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::PA1, Key::KeyState::Up));
	m_keyboard.emplace(std::pair(Key::Keyboard::OEM_CLEAR, Key::KeyState::Up));
}

Input::~Input()
{
}

void Input::is_key_down()
{
}

void Input::is_key_up()
{
}

int Input::get_mouse_x()
{
	return 0;
}

int Input::get_mouse_y()
{
	return 0;
}
