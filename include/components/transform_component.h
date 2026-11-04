#pragma once
#include "Core/defines.h"
#include "memory"


class TransformComponent {

public:
	// entt no mueve este componente al borrar otros: los punteros a el siguen siendo validos
	static constexpr bool in_place_delete = true;

	TransformComponent();
	TransformComponent(const Vec3& postion);
	TransformComponent(const Vec3& postion, const Vec3& scale);
	TransformComponent(const Vec3& postion, const Vec3& scale, const Vec3& rotation);
	TransformComponent(const TransformComponent&);
	TransformComponent(TransformComponent&&);
	~TransformComponent();

	void update();
	void force_update();


	// Rotation is in Radians
	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);

	void rotateXYZ(float x, float y, float z);
	void rotateXYZ(const Vec3& rotation);
	void rotateXYZ(const float rotation[3]);

	void set_position(const Vec3& position);
	void set_position(const float position[3]);

	void set_scale(const Vec3& scale);
	void set_scale(const float scale[3]);
	
	void set_rotation(const Vec3& rotation);
	void set_rotation(const float rotation[3]);

	// Cambia el padre sin tocar los valores locales: el objeto puede moverse en el mundo
	void set_parent(TransformComponent* parent);
	// Cambia el padre conservando la posicion en el mundo. Devuelve false si crearia un ciclo.
	bool set_parent_keep_world(TransformComponent* parent);
	TransformComponent* get_parent() const { return m_parent; }
	bool is_descendant_of(const TransformComponent* other) const;

	// Fija la matriz de mundo: calcula los valores locales respecto al padre
	bool set_world_matrix(const Mat4& world);

	Vec3& get_position();
	Vec3& get_rotation();
	Vec3& get_scale();

	const Vec3 get_forward_vector();
	const Vec3 get_right_vector();
	const Vec3 get_up_vector();


	const Mat4 get_transform();
	const Mat4 get_inverse_transform();

	float* get_transform_raw();

private:

	// Model matrix
	Mat4 m_transform;
	Mat4 m_inverse_transform;

	Vec3 m_position;
	Vec3 m_rotation;
	Vec3 m_scale;

	DirectX::XMFLOAT4X4 m_transform_float;
	float* m_transform_raw;

	TransformComponent* m_parent = nullptr;

	bool m_is_calculated;

	// Cambia cada vez que se recalcula la matriz: los hijos lo comparan para saber si su padre se ha movido
	unsigned int m_version = 0;
	unsigned int m_parent_version = 0;
};