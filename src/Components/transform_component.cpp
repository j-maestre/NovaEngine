
#include "components/transform_component.h"


TransformComponent::TransformComponent() : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
	
}


TransformComponent::TransformComponent(const Vec3& postion) : m_position(postion), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale) : m_position(postion), m_scale(scale), m_rotation(0.0f, 0.0f, 0.0f){
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale, const Vec3& rotation) : m_position(postion), m_scale(scale), m_rotation(rotation)
{
}

TransformComponent::TransformComponent(const TransformComponent& other){

}

TransformComponent::TransformComponent(TransformComponent&& other){

}

TransformComponent::~TransformComponent(){

}

void TransformComponent::update(){

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
