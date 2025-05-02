
#include "components/camera_component.h"
#include "Core/input.h"
#include "render/window.h"
#include "Core/engine.h"

CameraComponent::CameraComponent(const Input* input, Window* win){
	m_aspect_ratio = 16.0f / 9.0f;
	m_near = 0.1f;
	m_far = 1000.0f;

	m_fov = 90.0f;

	m_position = { 0.0f, 0.0f, 0.0f };
	m_pitch = -15.0f;
	m_yaw = 76.0f;

	m_input = input;
	
	m_direction = { 0.0f, 0.0f, -1.0f };

	WindowInfo* info = win->get_window_info();
	WindowProperties* props = win->get_window_properties();

	m_center_x = ((float)props->width) * 0.5f;
	m_center_y = ((float)props->height) * 0.5f;

	POINT center_tmp = {m_center_x, m_center_y};
	m_window_center = center_tmp;

	m_window_handle = info->window_handle;

	ClientToScreen(m_window_handle, &m_window_center);
	SetCursorPos(center_tmp.x, center_tmp.y);
	
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

	//printf("Pitch %f Yaw %f\n", m_pitch, m_yaw);
	Vec3 forward = { DirectX::XMVectorGetX(m_direction), DirectX::XMVectorGetY(m_direction), DirectX::XMVectorGetZ(m_direction) };
	Vec3 up(0.0f, -1.0f, 0.0f);

	FVector up_vec({ 0.0f, 1.0f, 0.0f });
	FVector right = DirectX::XMVector3Cross(m_direction, up_vec);

	Vec3 right_float = { DirectX::XMVectorGetX(right),DirectX::XMVectorGetY(right), DirectX::XMVectorGetZ(right) };

	const float speed = m_speed * dt;


	if (m_input->is_key_pressed(Key::Keyboard::W)) {
		
		m_position.x += forward.x * m_movement_speed * dt;
		m_position.y += forward.y * m_movement_speed * dt;
		m_position.z += forward.z * m_movement_speed * dt;
	}
	if (m_input->is_key_pressed(Key::Keyboard::S)) {
		m_position.x -= forward.x * m_movement_speed * dt;
		m_position.y -= forward.y * m_movement_speed * dt;
		m_position.z -= forward.z * m_movement_speed * dt;
	}
	
	if (m_input->is_key_pressed(Key::Keyboard::D)) {
		m_position.x -= right_float.x * m_movement_speed * dt;
		m_position.y -= right_float.y * m_movement_speed * dt;
		m_position.z -= right_float.z * m_movement_speed * dt;
	}
	
	if (m_input->is_key_pressed(Key::Keyboard::A)) {
		m_position.x += right_float.x * m_movement_speed * dt;
		m_position.y += right_float.y * m_movement_speed * dt;
		m_position.z += right_float.z * m_movement_speed * dt;
	}

	float MouseX = static_cast<float>(m_input->get_mouse_x());
	float MouseY = static_cast<float>(m_input->get_mouse_y());

	POINT center_tmp = { m_center_x, m_center_y };
	POINT mouse_pos = {MouseX, MouseY};
	ScreenToClient(m_window_handle, &mouse_pos);

	


	//if (m_input->is_key_pressed(Key::Keyboard::CONTROL)){

		if (m_first_move)
		{
			if(MouseX != 0.0f && MouseY != 0.0f)m_first_move = false;
			MouseX = center_tmp.x;
			MouseY = center_tmp.y;
			m_last_mouse_x = MouseX;
			m_last_mouse_y = MouseY;
			SetCursorPos(m_center_x, m_center_y);

			//m_Pitch = asin(forward.y / forward.length());
			//m_Yaw = asin(forward.x / (cos(m_Pitch) * forward.length()));
			
		}

		//float OffsetX = MouseX - m_last_mouse_x;
		//float OffsetY = MouseY - m_last_mouse_y;
		float OffsetX = static_cast<float>(MouseX - m_center_x);
		float OffsetY = static_cast<float>(MouseY - m_center_y);

		OffsetX *= m_mouse_sensitivity * dt;
		OffsetY *= m_mouse_sensitivity * dt;

		m_yaw += OffsetX;
		m_pitch -= OffsetY;

		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		if (m_yaw > 360.0f)
			m_yaw -= 360.0f;
		if (m_yaw < 0.0f)
			m_yaw += 360.0f;

		forward.x = cosf(degToRad(m_yaw)) * cosf(degToRad(m_pitch));
		forward.y = sinf(degToRad(m_pitch));
		forward.z = sinf(degToRad(m_yaw)) * cosf(degToRad(m_pitch));

		m_direction = DirectX::XMVector3Normalize(FVector({ forward.x, forward.y, forward.z, 0.0f}));
		//printf("Forward: X:%f Y:%f Z:%f\n", m_direction.x, m_direction.y, m_direction.z);
	//}

	m_last_mouse_x = MouseX;
	m_last_mouse_y = MouseY;

	ClientToScreen(m_window_handle, &center_tmp);
	SetCursorPos(center_tmp.x, center_tmp.y);

}



void CameraComponent::update_projection_matrix(){

	// Left-Handle more common un DirectX
	m_projection = DirectX::XMMatrixPerspectiveFovLH(degToRad(m_fov), m_aspect_ratio, m_near, m_far);
}

void CameraComponent::update_view_matrix(){


	//if (m_Data->MustRecalculeView || bForce){

		//m_Data->MustRecalculeView = false;
		
	 
	FVector position({ m_position.x, m_position.y, m_position.z, 1.0f });
	FVector direction({ DirectX::XMVectorGetX(m_direction) * -1.0f, DirectX::XMVectorGetY(m_direction), DirectX::XMVectorGetZ(m_direction), 0.0f });
	FVector up({ 0.0f, 1.0f, 0.0f, 0.0f });


		FVector cameraRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(direction, up));
		FVector cameraUp = DirectX::XMVector3Cross(cameraRight, direction);

		m_up.x = DirectX::XMVectorGetX(cameraUp);
		m_up.y = DirectX::XMVectorGetY(cameraUp);
		m_up.z = DirectX::XMVectorGetZ(cameraUp);

		m_right.x = DirectX::XMVectorGetX(cameraRight);
		m_right.y = DirectX::XMVectorGetY(cameraRight);
		m_right.z = DirectX::XMVectorGetZ(cameraRight);

		
		m_direction = DirectX::XMVector3Normalize(direction);
		FVector eye = DirectX::XMVectorAdd(position, m_direction);
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
