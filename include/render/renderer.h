#pragma once
#include "memory"
#include <Core/defines.h>
#include <Core/engine.h>
#include <components/camera_component.h>
#include <components/transform_component.h>
#include <components/mesh_component.h>
#include <render/material.h>
#include <Core/ECS.h>

class Renderer {


	enum classCullMode {
		Front,
		Back,
		Always,
		None
	};



public:
	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&);
	~Renderer();

	bool init_pipeline(Window* win);
	void active_shader(ShaderType type);

	void render_forward(EntityComponentSystem& ecs);

	void set_cull_mode();

	void set_camera(CameraComponent* cam);

	void release();
private:

	void clear_depth();
	void compile_shader(std::string path);
	void resize(unsigned int width, unsigned int height);
	__forceinline void render_mesh_internal(CameraConstantBuffer* camera_buffer, TransformComponent& trans, Mesh& m);

	friend class Window;

	ShaderFiles m_shader_files;
	Engine* m_engine_ptr;
	Window* m_window;


	ID3D11BlendState* m_blend_state_overwrite;
	ID3D11BlendState* m_blend_state_additive;
	unsigned int m_blend_mask = 0xffffffff;
	float m_blend_factor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	//ID3D11BlendState* m_blend_state_opaque;

	D3D11_BUFFER_DESC m_cube_index_buffer_desc; // tmp
	ID3D11Buffer* m_cube_index_buffer; // tmp

	ID3D11Texture2D* m_backbuffer_texture;

	D3D11_BUFFER_DESC m_buffer_description;
	D3D11_BUFFER_DESC m_buffer_description_cube;

	D3D11_BUFFER_DESC m_cam_constant_buffer;
	ID3D11Buffer* m_pVBufferConstantCamera;

	ID3D11Buffer* m_pVBufferLight;

	ID3D11Buffer* m_pVBuffer;

	ID3D11InputLayout* m_pLayout;
	
	ID3D11SamplerState* m_sampler_state;
	D3D11_SAMPLER_DESC m_sampler_desc;

	ID3D11Texture2D* m_depth_buffer;
	ID3D11DepthStencilView* m_depth_stencil_view;
	ID3D11DepthStencilState* m_depth_stencil_state;

	CameraComponent* m_cam;

	bool m_isInitialized;
	
};
