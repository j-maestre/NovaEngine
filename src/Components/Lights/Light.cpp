#include "components/Lights/Light.h"

Light::Light() : m_view(), m_projection(), m_view_projection(){

	m_cast_shadows = false;
	m_enabled = true;
	m_base_color = {1.0f, 1.0f, 1.0f};
	m_is_calculated = false;
}

Light::Light(float aspect_ratio) : m_view(), m_projection(), m_view_projection(){

	m_cast_shadows = false;
	m_enabled = true;
	m_base_color = {1.0f, 1.0f, 1.0f};
	m_is_calculated = false;
	m_aspect_ratio = aspect_ratio;
}

Light::Light(const Light&){

}

Light::Light(Light&&){

}

Light::~Light(){

}

void Light::update(){

	if (m_is_calculated) return;
	/*

	FVector position({ m_position.x, m_position.y, m_position.z, 1.0f });
	FVector direction({ m_direction.x, m_direction.y, m_direction.z, 0.0f });
	FVector up({ 0.0f, 1.0f, 0.0f, 0.0f });


	FVector cameraRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(direction, up));
	FVector cameraUp = DirectX::XMVector3Cross(cameraRight, direction);

	m_up.x = DirectX::XMVectorGetX(cameraUp);
	m_up.y = DirectX::XMVectorGetY(cameraUp);
	m_up.z = DirectX::XMVectorGetZ(cameraUp);

	m_right.x = DirectX::XMVectorGetX(cameraRight);
	m_right.y = DirectX::XMVectorGetY(cameraRight);
	m_right.z = DirectX::XMVectorGetZ(cameraRight);


	FVector direction_normalized = DirectX::XMVector3Normalize(direction);
	FVector eye = DirectX::XMVectorAdd(position, direction_normalized);
	m_view = DirectX::XMMatrixLookAtLH(position, eye, up);
	m_direction = { DirectX::XMVectorGetX(direction_normalized),DirectX::XMVectorGetY(direction_normalized) ,DirectX::XMVectorGetZ(direction_normalized) };
	
	m_view_projection = m_view * m_projection;

	*/
	m_is_calculated = true;

}

const Mat4* Light::get_view(){

	update();
	return &m_view;
}

const Mat4* Light::get_projection(){
	update();
	return &m_projection;
}

void Light::set_aspect_ratio(float value){
	m_aspect_ratio = value;
}
