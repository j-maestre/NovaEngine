#pragma once
#include "Core/defines.h"
#include "Core/input.h"
#include "memory"


class CameraComponent {

public:
	CameraComponent(const Input* input, Window* win);
	CameraComponent(const CameraComponent&);
	CameraComponent(CameraComponent&&);
	~CameraComponent();

	void update();
	void fly(float dt);

	void set_position(Vec3 pos) { m_position = pos; }
	Vec3 get_position() { return m_position; }

	float get_near() { return m_near; }
	float get_far() { return m_far; }
	float get_pitch() { return m_pitch; }
	float get_yaw() { return m_yaw; }
	float get_speed() { return m_speed; }
	float get_movement_speed() { return m_movement_speed; }
	float get_mouse_sensitivity() { return m_mouse_sensitivity; }

	void set_near(float value) { m_near = value; }
	void set_far(float value) { m_far = value; }
	void set_pitch(float value) { m_pitch = value; }
	void set_yaw(float value) { m_yaw = value; }
	void set_speed(float value) { m_speed = value; }
	void set_movement_speed(float value) { m_movement_speed = value; }
	void set_mouse_sensitivity(float value) { m_mouse_sensitivity = value; }

	void set_aspect_ratio(float value);

	const Mat4* get_view() noexcept { return &m_view; }
	const Mat4* get_projection() noexcept { return &m_projection; }
	const Mat4* get_view_projection() noexcept { return &m_view_projection; }
	



private:
	void update_view_matrix();
	void update_projection_matrix();

	WindowProperties* m_window_props;
	Window* m_win;

	HWND m_window_handle;
	POINT m_window_center;

	const Input* m_input;

	Mat4 m_view;
	Mat4 m_projection;
	Mat4 m_view_projection;	// View * projection combinated

	FVector m_direction;
	Vec3 m_up;
	Vec3 m_right;

	float m_center_x;
	float m_center_y;

	float m_pitch = 0.0f;
	float m_yaw;

	float m_aspect_ratio;

	float m_fov;
	float m_near;
	float m_far;

	Vec3 m_position;

	float m_speed = 1.0f;
	float m_movement_speed = 5.0f;
	float m_mouse_sensitivity = 40.0f;
	float m_last_mouse_x = 0.0f;
	float m_last_mouse_y = 0.0f;

	bool m_first_move = true;

	bool m_last_start_movement = false;
};