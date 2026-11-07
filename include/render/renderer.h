#pragma once
#include "memory"
#include <Core/defines.h>
#include <Core/engine.h>
#include <components/camera_component.h>
#include <components/transform_component.h>
#include <components/mesh_component.h>
#include <components/reflection_probe_component.h>
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

	void update_visible_meshes(EntityComponentSystem& ecs);
	void rebuild_depth_geometry();
	void depth_pass(EntityComponentSystem& ecs);
	void pick_entity(EntityComponentSystem& ecs, int x, int y);
	void update_light_buffer(EntityComponentSystem& ecs);
	void draw_skybox();
	void draw_skybox_to(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, const Mat4& view, const Mat4& proj, const Vec3& position);

	// Sondas de reflexion
	void init_reflection_probes();
	void release_reflection_probes();
	void update_reflection_probes(EntityComponentSystem& ecs);
	void capture_probe(unsigned int slot, const Vec3& position);

	void clear_shader_reources(int size = 5);
	__forceinline void clear_rtv(int size);
	__forceinline void clear_srv(int size);

	__forceinline void set_viewport(unsigned int width, unsigned int height);


	void create_backbuffers();
	void create_deferred_resources(unsigned int width, unsigned int height);
	void create_ssao_targets();
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
	SSAOConstantBuffer m_constant_buffer_ssao;

	ID3D11Buffer* m_pVBuffer_full_triangle;

	// Luces del pase deferred
	ID3D11Buffer* m_light_buffer = nullptr;
	ID3D11ShaderResourceView* m_light_buffer_srv = nullptr;
	ID3D11Buffer* m_pVBuffer_light_pass = nullptr;
	ID3D11Buffer* m_pVBuffer_composite = nullptr;
	ID3D11Buffer* m_pVBuffer_fxaa = nullptr;

	// Seleccion con clic
	ID3D11Buffer* m_pVBuffer_entity_id = nullptr;
	ID3D11Texture2D* m_entity_id_staging = nullptr;

	GpuProfiler m_gpu_profiler;
	std::vector<GPULight> m_gpu_lights;
	float m_time = 0.0f;	// Segundos desde el arranque, para lo que se anima en el render

	// Lo que se dibuja este frame: se calcula una vez y lo usan el prepass y el G-Buffer
	struct VisibleMesh {
		TransformComponent* transform;
		Mesh* mesh;
		unsigned int slot;		// Posicion de la malla en la geometria del prepass
	};
	std::vector<VisibleMesh> m_visible_meshes;
	std::vector<VisibleMesh> m_scene_meshes;	// Todas, visibles o no: las sondas ven en todas direcciones

	// Sondas de reflexion. Cada sonda tiene un hueco fijo en el array de cubos.
	struct ProbeSlot {
		bool captured = false;
		Vec3 position = { 0.0f, 0.0f, 0.0f };	// Desde donde se capturo
	};
	ProbeSlot m_probe_slots[MAX_REFLECTION_PROBES];
	// Captura en bruto: un cubo con todos sus mips, que lee el prefiltrado
	ID3D11Texture2D* m_probe_capture_texture = nullptr;
	ID3D11RenderTargetView* m_probe_capture_rtv[6] = {};
	ID3D11ShaderResourceView* m_probe_capture_srv = nullptr;
	ID3D11Texture2D* m_probe_depth_texture = nullptr;
	ID3D11DepthStencilView* m_probe_dsv = nullptr;
	// Prefiltradas: un array de cubos, un mip por rugosidad
	ID3D11Texture2D* m_probe_array_texture = nullptr;
	ID3D11RenderTargetView* m_probe_array_rtv[MAX_REFLECTION_PROBES * 6 * PROBE_MIPS] = {};
	ID3D11ShaderResourceView* m_probe_array_srv = nullptr;
	ID3D11Buffer* m_pVBuffer_probes = nullptr;
	ID3D11Buffer* m_pVBuffer_probe_prefilter = nullptr;
	Scene* m_probe_scene = nullptr;
	std::string m_probe_scene_path;
	float m_probe_scene_time = 0.0f;		// Segundos desde que se abrio la escena
	bool m_probe_was_loading = false;
	size_t m_probe_geometry_key = 0;
	unsigned int m_probe_realtime_next = 0;

	// Prepass en un solo draw call. Cada malla de cada entidad copia sus posiciones en un buffer comun,
	// y cada frame el index buffer se rellena solo con los rangos de las mallas visibles.
	struct DepthVertex {
		float x, y, z;
		uint32 object_index;
	};
	struct DepthRange {
		unsigned int first_index;
		unsigned int index_count;
	};
	std::vector<Mesh*> m_depth_all_meshes;					// Todas, en orden de slot
	std::vector<DirectX::XMFLOAT4X4> m_depth_matrices;		// Una por slot
	std::vector<unsigned int> m_depth_indices;				// Todas, con el offset ya sumado
	std::vector<DepthRange> m_depth_ranges;					// Una por slot
	std::vector<unsigned int> m_depth_uploaded_slots;		// Lo que hay ahora mismo en el index buffer
	size_t m_depth_geometry_key = 0;						// Si cambia, hay que reconstruir la geometria
	size_t m_depth_built_key = 0;
	unsigned int m_depth_index_count = 0;
	ID3D11Buffer* m_depth_vertex_buffer = nullptr;
	ID3D11Buffer* m_depth_index_buffer = nullptr;
	ID3D11Buffer* m_depth_matrix_buffer = nullptr;
	ID3D11ShaderResourceView* m_depth_matrix_srv = nullptr;
	ID3D11InputLayout* m_pLayout_depth_batch = nullptr;
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
	unsigned int m_target_width = 0;
	unsigned int m_target_height = 0;
	bool m_ssao_half_built = false;		// A que resolucion estan creadas ahora las texturas del SSAO
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
