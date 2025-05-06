#pragma once
#include "memory"
#include "Core/defines.h"



class Light {

public:
	Light();
	Light(float aspect_ratio);
	Light(const Light&);
	Light(Light&&);
	~Light();

	void update();
	const Mat4* get_view();
	const Mat4* get_projection();

	void set_aspect_ratio(float value);

protected:

	float m_aspect_ratio;
	bool m_cast_shadows;
	bool m_is_calculated;
	bool m_enabled = true;

	Mat4 m_view;
	Mat4 m_projection;
	Mat4 m_view_projection;

	Vec3 m_position;
	Vec3 m_direction;
	Vec3 m_rotation;

	Vec3 m_up;
	Vec3 m_right;

	Vec3 m_base_color;

private:


};
