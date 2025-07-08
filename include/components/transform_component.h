#pragma once
#include "Core/defines.h"
#include "memory"


class TransformComponent {

public:
	TransformComponent();
	TransformComponent(const Vec3& postion);
	TransformComponent(const Vec3& postion, const Vec3& scale);
	TransformComponent(const Vec3& postion, const Vec3& scale, const Vec3& rotation);
	TransformComponent(const TransformComponent&);
	TransformComponent(TransformComponent&&);
	~TransformComponent();

	void update();
	void force_update();

	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);

	void rotateXYZ(float x, float y, float z);
	void rotateXYZ(const Vec3& rotation);

	void set_position(const Vec3& position);
	void set_scale(const Vec3& scale);
	void set_rotation(const Vec3& rotation);

	void set_parent(TransformComponent* parent);

	Vec3& get_position();
	Vec3& get_rotation();
	Vec3& get_scale();

	const Vec3 get_forward_vector();
	const Vec3 get_right_vector();
	const Vec3 get_up_vector();


	const Mat4 get_transform();
	const Mat4 get_inverse_transform();

private:

	// Model matrix
	Mat4 m_transform;
	Mat4 m_inverse_transform;

	Vec3 m_position;
	Vec3 m_rotation;
	Vec3 m_scale;

	TransformComponent* m_parent = nullptr;

	bool m_is_calculated;
};