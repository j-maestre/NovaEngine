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
	


private:
	void update_view_matrix();
	void update_projection_matrix();

	Mat4 m_view;
	Mat4 m_projection;
	Mat4 m_view_projection;	// View * projection combinated

	float m_pitch;
	float m_yaw;

	float m_aspect_ratio;

	float m_fov;
	float m_near;
	float m_far;

	Vec3 m_position;


};