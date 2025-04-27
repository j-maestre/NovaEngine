
#include "components/camera_component.h"
#include "Core/input.h"
#include "render/window.h"
#include "Core/engine.h"

CameraComponent::CameraComponent(const Input* input){
	m_aspect_ratio = 16.0f / 9.0f;
	m_near = 0.1f;
	m_far = 1000.0f;

	m_fov = 90.0f;

	m_position = { 0.0f, 0.0f, 0.0f };
	m_yaw = -90.0f;

	m_input = input;
}

CameraComponent::CameraComponent(const CameraComponent&)
{
}

CameraComponent::CameraComponent(CameraComponent&&)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::update(){
	update_view_matrix();
	update_projection_matrix();

	m_view_projection = (m_projection * m_view);
}

void CameraComponent::fly(){

	if (m_input->is_key_down(Key::Keyboard::CONTROL)) {

		Vec3 direction;
		direction.x = cosf(degToRad(m_yaw));
		direction.x = sinf(degToRad(m_pitch));
		direction.z = sinf(degToRad(m_yaw));

		float mouse_x = m_input->get_mouse_x();
		float mouse_y = m_input->get_mouse_y();

		float xoffset = (mouse_x - m_last_mouse_x) * m_mouse_sensitivity;
		float yoffset = (mouse_y - m_last_mouse_y) * m_mouse_sensitivity;

		m_yaw += xoffset;
		m_pitch += yoffset;


		m_last_mouse_x = mouse_x;
		m_last_mouse_y = mouse_y;
	}
}

void CameraComponent::update_view_matrix(){

	float pitch = degToRad(m_pitch);
	float yaw = degToRad(m_yaw);

	float cosPitch = cosf(pitch);
	float cosYaw = cosf(yaw);

	float sinPitch = sinf(pitch);
	float sinYaw = sinf(yaw);

	Vec3 xaxis = { cosYaw, 0, -sinYaw };
	Vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	Vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
	
	FVector xaxis_tmp({ xaxis.x, xaxis.y, xaxis.z, 0.0f });
	FVector yaxis_tmp({ yaxis.x, yaxis.y, yaxis.z, 0.0f });
	FVector zaxis_tmp({ zaxis.x, zaxis.y, zaxis.z, 0.0f });
	FVector pos_tmp({ m_position.x, m_position.y, m_position.z, 0.0f });
	
	FVector tmpX = DirectX::XMVector3Dot(xaxis_tmp, pos_tmp);
	FVector tmpY = DirectX::XMVector3Dot(yaxis_tmp, pos_tmp);
	FVector tmpZ = DirectX::XMVector3Dot(zaxis_tmp, pos_tmp);

	FVector row4({ DirectX::XMVectorGetX(tmpX) * -1.0f, DirectX::XMVectorGetX(tmpY) * -1.0f, DirectX::XMVectorGetX(tmpZ) * -1.0f, 1.0f });
	
	m_view = {
		xaxis.x, yaxis.x, zaxis.x, 0.0f,
		xaxis.y, yaxis.y, zaxis.y, 0.0f,
		xaxis.z, yaxis.z, zaxis.z, 0.0f,
		DirectX::XMVectorGetX(row4), DirectX::XMVectorGetY(row4), DirectX::XMVectorGetZ(row4), 1.0f
	};

}

void CameraComponent::update_projection_matrix(){

	// Left-Handle more common un DirectX
	m_projection = DirectX::XMMatrixPerspectiveFovLH(degToRad(m_fov), m_aspect_ratio, m_near, m_far);
}
