#pragma once
#include "Core/defines.h"
#include "memory"


class CameraComponent {

public:
	CameraComponent();
	CameraComponent(const CameraComponent&);
	CameraComponent(CameraComponent&&);
	~CameraComponent();

	void update();

	void set_position(Vec3 pos) { m_position = pos; }
	Vec3 get_position() { return m_position; }

	const Mat4* get_view() noexcept { return &m_view; }
	const Mat4* get_projection() noexcept { return &m_projection; }
	const Mat4* get_view_projection() noexcept { return &m_view_projection; }
	


private:
	void update_view_matrix();
	void update_projection_matrix();

	Mat4 m_view;
	Mat4 m_projection;
	Mat4 m_view_projection;	// View * projection combinated

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;

	float m_aspect_ratio;

	float m_fov;
	float m_near;
	float m_far;

	Vec3 m_position;


};