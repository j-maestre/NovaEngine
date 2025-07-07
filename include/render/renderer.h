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

	void enable_depth_prepass(bool enabled);

	void render_forward(EntityComponentSystem& ecs);
	void render_deferred(EntityComponentSystem& ecs);

	void set_draw_mode(DrawMode mode);
	void set_cull_mode();
	void set_camera(CameraComponent* cam);

	DrawMode get_current_draw_mode();

	void release();
private:

	void compile_shader(std::string path);
	__forceinline void clear_depth();
	__forceinline void clear_render_target();
	__forceinline void clear_full_quad();
	__forceinline void render_mesh_internal(CameraConstantBuffer* camera_buffer, TransformComponent& trans, Mesh& m);
	__forceinline void render_mesh_depth_only(CameraDepthPrePass* camera_buffer, TransformComponent& trans, Mesh& m);
	__forceinline void render_deferred_internal();
	__forceinline void draw_emissive();
	__forceinline void draw_emissive_downsample();

	void depth_pass(EntityComponentSystem& ecs);
	void draw_skybox();

	void clear_shader_reources(int size = 5);
	__forceinline void clear_rtv(int size);
	__forceinline void clear_srv(int size);

	__forceinline void set_viewport(unsigned int width, unsigned int height);


	void create_backbuffers();
	void create_deferred_resources(unsigned int width, unsigned int height);
	void release_deferred_resources();

	__forceinline void add_draw_call();

	friend class Window;

	ShaderFiles m_shader_files;
	Engine* m_engine_ptr;
	Window* m_window;

	ID3D11RasterizerState* m_current_raster;

	ID3D11BlendState* m_blend_state_overwrite;
	ID3D11BlendState* m_blend_state_additive;
	ID3D11BlendState* m_blend_off;
	ID3D11BlendState* m_blend_skybox;
	unsigned int m_blend_mask = 0xffffffff;
	float m_blend_factor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	//ID3D11BlendState* m_blend_state_opaque;

	D3D11_BUFFER_DESC m_cube_index_buffer_desc; // tmp
	ID3D11Buffer* m_cube_index_buffer; // tmp

	ID3D11Texture2D* m_backbuffer_texture;
	
	// Emissive
	ID3D11Texture2D* m_emissive_texture;
	ID3D11ShaderResourceView* m_emissive_SRV = nullptr;
	float m_clear_emissive_color[4];

	DeferredResources m_deferred_resources;

	// Full screen quad
	ID3D11Texture2D* m_quad_texture;
	ID3D11ShaderResourceView* m_quad_SRV = nullptr;
	ID3D11RenderTargetView* m_quad_RTV = nullptr;


	D3D11_BUFFER_DESC m_buffer_description;
	D3D11_BUFFER_DESC m_buffer_description_cube;

	D3D11_BUFFER_DESC m_cam_constant_buffer;
	ID3D11Buffer* m_pVBufferConstantCamera;

	D3D11_BUFFER_DESC m_cam_depth_only_constant_buffer;
	ID3D11Buffer* m_pVBuffer_constant_camera_depth_only;

	D3D11_BUFFER_DESC m_cam_deferred_constant_buffer;
	ID3D11Buffer* m_pVBufferDeferredConstantCamera;

	D3D11_BUFFER_DESC m_emissive_constant_buffer_desc;
	ID3D11Buffer* m_pVBuffer_emissive_constant_buffer;

	D3D11_BUFFER_DESC m_skybox_constant_buffer_desc;
	ID3D11Buffer* m_pVBuffer_skybox_buffer;

	ID3D11Buffer* m_pVBufferLight;

	ID3D11Buffer* m_pVBuffer;
	ID3D11Buffer* m_pVBuffer_full_triangle;

	ID3D11InputLayout* m_pLayout;
	ID3D11InputLayout* m_pLayout_deferred;
	ID3D11InputLayout* m_pLayout_skybox;
	ID3D11InputLayout* m_pLayout_depth;
	
	ID3D11SamplerState* m_sampler_state_emissive;
	ID3D11SamplerState* m_sampler_state;
	D3D11_SAMPLER_DESC m_sampler_desc;

	ID3D11Texture2D* m_depth_buffer;
	ID3D11DepthStencilView* m_depth_stencil_view;
	ID3D11DepthStencilState* m_depth_stencil_state;

	ID3D11DepthStencilState* m_depth_stencil_state_skybox;
	ID3D11DepthStencilState* m_depth_only_state;

	CameraComponent* m_cam;

	VertexQuad m_fs_quad[3];
	D3D11_BUFFER_DESC m_buffer_description_full_triangle;
	bool m_isInitialized;
	bool m_bloom_active = true;
	bool m_depth_prepass = true;

	int m_buffer_index = 0;
	std::string m_pixel_shader_model;
	std::string m_vertex_shader_model;

	DrawMode m_current_draw_mode;

	float m_skybox_vertices[36 * 3] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	
};
