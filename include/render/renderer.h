#pragma once
#include "memory"
#include <Core/defines.h>
#include <Core/engine.h>
#include <components/camera_component.h>
#include <components/transform_component.h>
#include <Core/ECS.h>

class Renderer {



public:
	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&);
	~Renderer();

	bool init_pipeline(Engine* engine);
	void active_shader(ShaderType type);

	void render_forward(EntityComponentSystem& ecs, Texture t);

	void upload_triangle();
	void draw_triangle();
	void draw_cube();

	void set_camera(CameraComponent* cam);

	void release();
private:

	void compile_shader(std::string path);
	ShaderFiles m_shader_files;
	Engine* m_engine_ptr;

	D3D11_BUFFER_DESC m_cube_index_buffer_desc; // tmp
	ID3D11Buffer* m_cube_index_buffer; // tmp

	D3D11_BUFFER_DESC m_buffer_description;
	D3D11_BUFFER_DESC m_buffer_description_cube;

	D3D11_BUFFER_DESC m_cam_constant_buffer;
	ID3D11Buffer* m_pVBufferConstantCamera;

	ID3D11Buffer* m_pVBuffer;
	ID3D11Buffer* m_pVBufferCube;

	ID3D11InputLayout* m_pLayout;
	
	ID3D11SamplerState* m_sampler_state;
	D3D11_SAMPLER_DESC m_sampler_desc;



	CameraComponent* m_cam;

	bool m_isInitialized;
	
};
