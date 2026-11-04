#pragma once
#include "memory"
#include <Core/defines.h>
#include <Core/engine.h>
#include <components/camera_component.h>
#include <components/transform_component.h>
#include <components/mesh_component.h>
#include <render/material.h>
#include <render/gpu_profiler.h>
#include <Core/ECS.h>

class Renderer {
public:
	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	~Renderer();

	bool init_pipeline(Window* win);
	void active_shader(ShaderType type);

	void render_forward(EntityComponentSystem& ecs);
	void render_deferred(EntityComponentSystem& ecs);

	void set_draw_mode(DrawMode mode);
	void set_camera(CameraComponent* cam);

	DrawMode get_current_draw_mode();

	void release();
private:

	__forceinline void clear_depth();
	__forceinline void clear_render_target();
	__forceinline void render_mesh_internal(CameraConstantBuffer* camera_buffer, TransformComponent& trans, Mesh& m);
	__forceinline void render_mesh_depth_only(CameraDepthPrePass* camera_buffer, TransformComponent& trans, Mesh& m);

	/*** Post process techniques ***/
	__forceinline void draw_emissive_downsample();
	__forceinline void draw_ssao(Mat4* projection, Mat4* view, Mat4* projection_inverse);
	__forceinline void draw_ssao_blur();
	/*******************************/

	void depth_pass(EntityComponentSystem& ecs);
	void update_light_buffer(EntityComponentSystem& ecs);
	void draw_skybox();

	void clear_shader_reources(int size = 5);
	__forceinline void clear_rtv(int size);
	__forceinline void clear_srv(int size);

	__forceinline void set_viewport(unsigned int width, unsigned int height);


	void create_backbuffers();
	void create_deferred_resources(unsigned int width, unsigned int height);
	void release_deferred_resources();

	void init_ssao();

	__forceinline void add_draw_call();

	friend class Window;

	ShaderFiles m_shader_files;
	Engine* m_engine_ptr;
	Window* m_window;

	ID3D11RasterizerState* m_current_raster;

	ID3D11BlendState* m_blend_state_overwrite;
	ID3D11BlendState* m_blend_state_additive;
	//ID3D11BlendState* m_blend_state_opaque;

	ID3D11Texture2D* m_backbuffer_texture = nullptr;
	
	// Clear colors
	float m_clear_emissive_color[4];
	float m_clear_depth_color[4];

	DeferredResources m_deferred_resources{};

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

	D3D11_BUFFER_DESC m_ssao_constant_buffer_desc;
	ID3D11Buffer* m_pVBuffer_ssao;
	ID3D11Buffer* m_pVBuffer_blend_ssao;
	SSAOConstantBuffer m_constant_buffer_ssao;
	SSAOBlendConstantBuffer m_ssao_blend_constant_buffer;

	ID3D11Buffer* m_pVBuffer_full_triangle;

	// Luces del pase deferred
	ID3D11Buffer* m_light_buffer = nullptr;
	ID3D11ShaderResourceView* m_light_buffer_srv = nullptr;
	ID3D11Buffer* m_pVBuffer_light_pass = nullptr;
	ID3D11Buffer* m_pVBuffer_composite = nullptr;
	GpuProfiler m_gpu_profiler;
	std::vector<GPULight> m_gpu_lights;
	unsigned int m_light_buffer_capacity = 0;

	ID3D11InputLayout* m_pLayout;
	ID3D11InputLayout* m_pLayout_deferred;
	ID3D11InputLayout* m_pLayout_skybox;
	ID3D11InputLayout* m_pLayout_depth;
	
	ID3D11SamplerState* m_sampler_state_emissive;
	ID3D11SamplerState* m_sampler_state;
	ID3D11SamplerState* m_sampler_state_noise_ssao;
	D3D11_SAMPLER_DESC m_sampler_desc;

	ID3D11Texture2D* m_depth_buffer;
	ID3D11DepthStencilView* m_depth_stencil_view;
	ID3D11DepthStencilState* m_depth_stencil_state;

	ID3D11DepthStencilState* m_depth_only_state;
	ID3D11DepthStencilState* m_depth_stencil_ssao;



	CameraComponent* m_cam;

	VertexQuad m_fs_quad[3];
	D3D11_BUFFER_DESC m_buffer_description_full_triangle;

	RenderInfo m_render_info_parameters;
	bool m_isInitialized;
	std::string m_pixel_shader_model;
	std::string m_vertex_shader_model;

	DrawMode m_current_draw_mode;

	ID3D11Texture2D* m_noise_texture = nullptr;
	ID3D11ShaderResourceView* m_noise_srv = nullptr;
	std::vector<float> m_ssao_noise_random;

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
