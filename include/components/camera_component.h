#pragma once
#include "Core/defines.h"
#include "Core/input.h"
#include "memory"


class CameraComponent {

public:
	CameraComponent(const Input* input);
	CameraComponent(const CameraComponent&);
	CameraComponent(CameraComponent&&);
	~CameraComponent();

	void update();
	void fly(float dt);

	void set_position(Vec3 pos) { m_position = pos; }
	Vec3 get_position() { return m_position; }

	const Mat4* get_view() noexcept { return &m_view; }
	const Mat4* get_projection() noexcept { return &m_projection; }
	const Mat4* get_view_projection() noexcept { return &m_view_projection; }
	


private:
	void update_view_matrix();
	void update_projection_matrix();



	const Input* m_input;

	Mat4 m_view;
	Mat4 m_projection;
	Mat4 m_view_projection;	// View * projection combinated

	Vec3 m_direction;
	Vec3 m_up;
	Vec3 m_right;

	float m_pitch = 0.0f;
	float m_yaw;

	float m_aspect_ratio;

	float m_fov;
	float m_near;
	float m_far;

	Vec3 m_position;

	float m_speed = 1.0f;
	float m_mouse_sensitivity = 0.1f;
	float m_last_mouse_x = 0.0f;
	float m_last_mouse_y = 0.0f;

	bool m_first_move = true;
};