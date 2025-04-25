
#include "components/transform_component.h"


TransformComponent::TransformComponent() : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
	m_is_calculated = false;
}


TransformComponent::TransformComponent(const Vec3& postion) : m_position(postion), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale) : m_position(postion), m_scale(scale), m_rotation(0.0f, 0.0f, 0.0f){
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale, const Vec3& rotation) : m_position(postion), m_scale(scale), m_rotation(rotation){
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const TransformComponent& other){

}

TransformComponent::TransformComponent(TransformComponent&& other){

}

TransformComponent::~TransformComponent(){

}

void TransformComponent::update(){

	if (m_is_calculated) return;

	Mat4 scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	Mat4 rotationX = DirectX::XMMatrixRotationX(m_rotation.x);
	Mat4 rotationY = DirectX::XMMatrixRotationY(m_rotation.y);
	Mat4 rotationZ = DirectX::XMMatrixRotationZ(m_rotation.z);
	Mat4 rotation = rotationZ * rotationY * rotationX;

	Mat4 translation = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	
	// Model construction
	m_transform = scale * rotation * translation;
	
	// Inverse model
	m_inverse_transform = DirectX::XMMatrixInverse(nullptr, m_transform);


	m_is_calculated = true;
}

void TransformComponent::rotateX(float angle){

}

void TransformComponent::rotateY(float angle){

}

void TransformComponent::rotateZ(float angle){

}

void TransformComponent::rotateXYZ(float x, float y, float z){

	m_rotation = Vec3(x,y,z);
}

void TransformComponent::rotateXYZ(const Vec3& rotation){
	m_rotation = rotation;
}

void TransformComponent::set_position(const Vec3& position){

	m_position = position;
}

void TransformComponent::set_scale(const Vec3& scale){

	m_scale = scale;
}

const Vec3 TransformComponent::get_position(){

	return m_position;
}

const Vec3 TransformComponent::get_rotation(){

	return m_rotation;
}

const Vec3 TransformComponent::get_scale()
{
	return m_scale;
}

const Vec3 TransformComponent::get_forward_vector()
{
	return Vec3();
}

const Vec3 TransformComponent::get_right_vector()
{
	return Vec3();
}

const Vec3 TransformComponent::get_up_vector()
{
	return Vec3();
}

const Mat4 TransformComponent::get_transform()
{
	return m_transform;
}

const Mat4 TransformComponent::get_inverse_transform()
{
	return m_inverse_transform;
}
