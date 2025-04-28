
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
	m_pitch = 4.23f;
	m_yaw = -56.0f;

	m_input = input;
	
	m_direction = { 0.0f, 0.0f, -1.0f };
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


void CameraComponent::fly(float dt){
	//glm::vec3 position = glm::make_vec3(GetPosition());
	Vec3 forward = m_direction;
	Vec3 up(0.0f, -1.0f, 0.0f);

	const float speed = m_speed * dt;


	const float MouseX = static_cast<float>(m_input->get_mouse_x());
	const float MouseY = static_cast<float>(m_input->get_mouse_y());

	//if (m_input->is_key_down(Key::Keyboard::CONTROL)){

		if (m_first_move)
		{
			m_last_mouse_x = MouseX;
			m_last_mouse_y = MouseY;
			//m_Pitch = asin(forward.y / forward.length());
			//m_Yaw = asin(forward.x / (cos(m_Pitch) * forward.length()));
			m_first_move = false;
		}

		float OffsetX = MouseX - m_last_mouse_x;
		float OffsetY = MouseY - m_last_mouse_y;

		OffsetX *= m_mouse_sensitivity;
		OffsetY *= m_mouse_sensitivity;

		m_yaw += OffsetX;
		m_pitch -= OffsetY;

		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		forward.x = cosf(degToRad(m_yaw)) * cosf(degToRad(m_pitch));
		forward.y = sinf(degToRad(m_pitch));
		forward.z = sinf(degToRad(m_yaw)) * cosf(degToRad(m_pitch));

		m_direction = forward;
	//}

	m_last_mouse_x = MouseX;
	m_last_mouse_y = MouseY;

	//SetPosition(position.x, position.y, position.z);
}



void CameraComponent::update_projection_matrix(){

	// Left-Handle more common un DirectX
	m_projection = DirectX::XMMatrixPerspectiveFovLH(degToRad(m_fov), m_aspect_ratio, m_near, m_far);
}

void CameraComponent::update_view_matrix(){


	//if (m_Data->MustRecalculeView || bForce){

		//m_Data->MustRecalculeView = false;
		
	 
	FVector position({ m_position.x, m_position.y, m_position.z, 1.0f });
	FVector direction({ -m_direction.x, m_direction.y, m_direction.z, 0.0f });
	FVector up({ 0.0f, 1.0f, 0.0f, 0.0f });


		
		FVector cameraRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, direction));
		FVector cameraUp = DirectX::XMVector3Cross(direction, cameraRight);

		m_up.x = DirectX::XMVectorGetX(cameraUp);
		m_up.y = DirectX::XMVectorGetY(cameraUp);
		m_up.z = DirectX::XMVectorGetZ(cameraUp);

		m_right.x = DirectX::XMVectorGetX(cameraRight);
		m_right.y = DirectX::XMVectorGetY(cameraRight);
		m_right.z = DirectX::XMVectorGetZ(cameraRight);

		
		FVector dir_normalized = DirectX::XMVector3Normalize(direction);
		FVector eye = DirectX::XMVectorAdd(position, dir_normalized);
		m_view = DirectX::XMMatrixLookAtLH(position, eye, up);


		//m_Data->ProjectionViewMatrix = m_Data->ProjectionMatrix * m_Data->ViewMatrix;
	//}

}




/* Maybe for controller */
/*
void CameraComponent::update_view_matrix(){

	float pitch = degToRad(m_pitch);
	float yaw = degToRad(m_yaw);

	float cosPitch = cosf(pitch);
	float cosYaw = cosf(yaw);

	float sinPitch = sinf(pitch);
	float sinYaw = sinf(yaw);

	Vec3 xaxis = { cosYaw, 0.0f, -sinYaw };
	Vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	Vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

	FVector xaxis_tmp({ xaxis.x, xaxis.y, xaxis.z, 0.0f });
	FVector yaxis_tmp({ yaxis.x, yaxis.y, yaxis.z, 0.0f });
	FVector zaxis_tmp({ zaxis.x, zaxis.y, zaxis.z, 0.0f });
	FVector pos_tmp({ m_position.x, m_position.y, m_position.z, 0.0f });

	FVector tmpX = DirectX::XMVector3Dot(xaxis_tmp, pos_tmp);
	FVector tmpY = DirectX::XMVector3Dot(yaxis_tmp, pos_tmp);
	FVector tmpZ = DirectX::XMVector3Dot(zaxis_tmp, pos_tmp);

	FVector row4({ DirectX::XMVectorGetX(tmpX) * -1.0f, DirectX::XMVectorGetY(tmpY) * -1.0f, DirectX::XMVectorGetZ(tmpZ) * -1.0f, 1.0f });

	m_view = {
		xaxis.x, yaxis.x, zaxis.x, 0.0f,
		xaxis.y, yaxis.y, zaxis.y, 0.0f,
		xaxis.z, yaxis.z, zaxis.z, 0.0f,
		DirectX::XMVectorGetX(row4), DirectX::XMVectorGetY(row4), DirectX::XMVectorGetZ(row4), 1.0f
	};

}
*/
