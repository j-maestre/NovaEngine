#include <render/renderer.h>
#include <d3dcompiler.h>
#include <algorithm>
#include <random>
#include "render/imgui/imgui_manager.h"


static inline bool CheckShaderError(HRESULT hr) {
	return SUCCEEDED(hr);
}

Renderer::Renderer() : m_shader_files(), m_sampler_desc{} {
	m_isInitialized = false;
	m_pVBuffer_full_triangle = nullptr;
	m_clear_emissive_color[0] = 0.0f;
	m_clear_emissive_color[1] = 0.0f;
	m_clear_emissive_color[2] = 0.0f;
	m_clear_emissive_color[3] = 0.0f;


	m_fs_quad[0] = { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} };
	m_fs_quad[1] = { {-1.0f,  3.0f, 0.0f}, {0.0f, -1.0f} };
	m_fs_quad[2] = { { 3.0f, -1.0f, 0.0f}, {2.0f, 1.0f} };

	m_pixel_shader_model = "ps_5_0";
	m_vertex_shader_model = "vs_5_0";
}

Renderer::~Renderer(){
	m_depth_buffer->Release();
	m_depth_stencil_view->Release();
	m_pVBufferConstantCamera->Release();
	m_pVBufferDeferredConstantCamera->Release();
	m_pVBuffer_ssao->Release();
	m_pVBuffer_full_triangle->Release();
	if (m_light_buffer_srv) m_light_buffer_srv->Release();
	if (m_light_buffer) m_light_buffer->Release();
	if (m_pVBuffer_light_pass) m_pVBuffer_light_pass->Release();
	if (m_pVBuffer_composite) m_pVBuffer_composite->Release();
	if (m_pVBuffer_fxaa) m_pVBuffer_fxaa->Release();
	if (m_depth_vertex_buffer) m_depth_vertex_buffer->Release();
	if (m_depth_index_buffer) m_depth_index_buffer->Release();
	if (m_depth_matrix_buffer) m_depth_matrix_buffer->Release();
	if (m_depth_matrix_srv) m_depth_matrix_srv->Release();
	if (m_pLayout_depth_batch) m_pLayout_depth_batch->Release();
	if (m_pVBuffer_entity_id) m_pVBuffer_entity_id->Release();
	if (m_entity_id_staging) m_entity_id_staging->Release();

	m_pLayout->Release();
	m_sampler_state->Release();
	m_depth_stencil_state->Release();
	release();

}

bool Renderer::init_pipeline(Window* win){

	m_engine_ptr = Engine::get_instance();
	m_window = win;

	// ComPtr: cada blob se libera al compilar otro en la misma variable y al salir de esta funcion
	ComPtr<ID3DBlob> VS;
	ComPtr<ID3DBlob> VS_deferred;
	ComPtr<ID3DBlob> VS_skybox;
	ComPtr<ID3DBlob> VS_depth;
	ComPtr<ID3DBlob> VS_depth_batch;
	ComPtr<ID3DBlob> PS;
	HRESULT hr = S_OK;

	const auto compile = [&](const wchar_t* path, const char* entry, const std::string& model, ComPtr<ID3DBlob>& blob) {
		ComPtr<ID3DBlob> error_msg;
		hr = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, blob.ReleaseAndGetAddressOf(), error_msg.GetAddressOf());
		m_isInitialized = SUCCEEDED(hr);
		if (!m_isInitialized && error_msg) {
			OutputDebugStringA(static_cast<const char*>(error_msg->GetBufferPointer()));
			printf("%s\n", static_cast<const char*>(error_msg->GetBufferPointer()));
		}
		return m_isInitialized;
	};
	
	// Forward rendering shaders

	if (!compile(L"data/shaders/forward/vs_common.hlsl", "VShader", m_vertex_shader_model, VS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_shader_files.VS_common);


	if (!compile(L"data/shaders/forward/ps_directional.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_directional);

	if (!compile(L"data/shaders/forward/ps_point.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_point);

	if (!compile(L"data/shaders/forward/ps_spot.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_spot);
	


	// Deferred rendering shaders

	
	// Vertex shader geometry pass
	if (!compile(L"data/shaders/deferred/vs_deferred.hlsl", "VShader", m_vertex_shader_model, VS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),NULL, &m_shader_files.VS_deferred);
	
	// Pixel shader geometry pass
	if (!compile(L"data/shaders/deferred/ps_deferred.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred);

	// Light pass vertex shader
	if (!compile(L"data/shaders/deferred/vs_deferred_common.hlsl", "VShader", m_vertex_shader_model, VS_deferred)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_deferred->GetBufferPointer(), VS_deferred->GetBufferSize(), NULL, &m_shader_files.VS_deferred_common);

	// Light pass pixel shader (todas las luces en un solo draw)
	if (!compile(L"data/shaders/deferred/ps_deferred_lights.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_lights);

	// Composite: tonemapping y gamma
	if (!compile(L"data/shaders/deferred/ps_deferred_composite.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_composite);

	// FXAA y dithering, lo ultimo antes del backbuffer
	if (!compile(L"data/shaders/deferred/ps_deferred_fxaa.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_fxaa);

	// Bloom downscaling
	if (!compile(L"data/shaders/deferred/ps_deferred_post_process_bloom_downscaling.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_emissive_downsample);
	
	
	
	// Skybox VS
	if (!compile(L"data/shaders/deferred/vs_deferred_skybox.hlsl", "VShader", m_vertex_shader_model, VS_skybox)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_skybox->GetBufferPointer(), VS_skybox->GetBufferSize(), NULL, &m_shader_files.VS_deferred_skybox);

	// Skybox PS
	if (!compile(L"data/shaders/deferred/ps_deferred_skybox.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_skybox);

	// SSAO PS
	if (!compile(L"data/shaders/deferred/ps_deferred_ssao.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_SSAO);

	// SSAO VS
	if (!compile(L"data/shaders/deferred/vs_deferred_ssao.hlsl", "VShader", m_vertex_shader_model, VS_deferred)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_deferred->GetBufferPointer(), VS_deferred->GetBufferSize(), NULL, &m_shader_files.VS_SSAO);


	// SSAO Blur
	if (!compile(L"data/shaders/deferred/ps_deferred_ssao_blur.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_SSAO_Blur_blend);

	// Sondas de reflexion: captura de la escena y prefiltrado por rugosidad
	if (!compile(L"data/shaders/deferred/ps_probe_capture.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_probe_capture);

	if (!compile(L"data/shaders/deferred/ps_probe_prefilter.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_probe_prefilter);

	// Seleccion con clic
	if (!compile(L"data/shaders/ps_entity_id.hlsl", "PShader", m_pixel_shader_model, PS)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_entity_id);

	// Depth prepass VS
	if (!compile(L"data/shaders/vs_depth_prepass.hlsl", "VShader", m_vertex_shader_model, VS_depth)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_depth->GetBufferPointer(), VS_depth->GetBufferSize(), NULL, &m_shader_files.VS_depth_prepass);

	// Depth prepass VS, toda la escena en un draw
	if (!compile(L"data/shaders/vs_depth_batch.hlsl", "VShader", m_vertex_shader_model, VS_depth_batch)) return false;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_depth_batch->GetBufferPointer(), VS_depth_batch->GetBufferSize(), NULL, &m_shader_files.VS_depth_batch);

	
	
	

	/**** Full Screen Triangle creation ****/
	ZeroMemory(&m_buffer_description_full_triangle, sizeof(m_buffer_description_full_triangle));
	m_buffer_description_full_triangle.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
	m_buffer_description_full_triangle.ByteWidth = sizeof(VertexQuad) * 3;
	m_buffer_description_full_triangle.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// Using as Vertex buffer
	m_buffer_description_full_triangle.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = m_fs_quad;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_buffer_description_full_triangle, &initData, &m_pVBuffer_full_triangle);

	
	/**** Camera & model constant buffer creation ****/
	ZeroMemory(&m_cam_constant_buffer, sizeof(m_cam_constant_buffer));
	m_cam_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_cam_constant_buffer.ByteWidth = sizeof(CameraConstantBuffer);
	m_cam_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_cam_constant_buffer.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cam_constant_buffer,NULL, &m_pVBufferConstantCamera);
	
	/**** Camera depth only buffer creation ****/
	ZeroMemory(&m_cam_depth_only_constant_buffer, sizeof(m_cam_depth_only_constant_buffer));
	m_cam_depth_only_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_cam_depth_only_constant_buffer.ByteWidth = sizeof(CameraDepthPrePass);
	m_cam_depth_only_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_cam_depth_only_constant_buffer.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cam_depth_only_constant_buffer,NULL, &m_pVBuffer_constant_camera_depth_only);

	// Camera deferred constant buffer creation
	ZeroMemory(&m_cam_deferred_constant_buffer, sizeof(m_cam_deferred_constant_buffer));
	m_cam_deferred_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_cam_deferred_constant_buffer.ByteWidth = sizeof(CameraDeferredConstantBuffer);
	m_cam_deferred_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_cam_deferred_constant_buffer.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cam_deferred_constant_buffer,NULL, &m_pVBufferDeferredConstantCamera);

	// Emissive postprocess constant buffer creation
	ZeroMemory(&m_emissive_constant_buffer_desc, sizeof(m_emissive_constant_buffer_desc));
	m_emissive_constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	m_emissive_constant_buffer_desc.ByteWidth = sizeof(EmissiveConstantBuffer);
	m_emissive_constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_emissive_constant_buffer_desc.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_emissive_constant_buffer_desc,NULL, &m_pVBuffer_emissive_constant_buffer);

	// SSAO constant buffer creation
	ZeroMemory(&m_ssao_constant_buffer_desc, sizeof(m_ssao_constant_buffer_desc));
	m_ssao_constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	m_ssao_constant_buffer_desc.ByteWidth = sizeof(SSAOConstantBuffer);
	m_ssao_constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_ssao_constant_buffer_desc.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_ssao_constant_buffer_desc,NULL, &m_pVBuffer_ssao);

	// Light pass constant buffer creation
	D3D11_BUFFER_DESC light_pass_desc = {};
	light_pass_desc.Usage = D3D11_USAGE_DEFAULT;
	light_pass_desc.ByteWidth = sizeof(LightPassConstantBuffer);
	light_pass_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&light_pass_desc, NULL, &m_pVBuffer_light_pass);

	// Composite constant buffer creation
	D3D11_BUFFER_DESC composite_desc = {};
	composite_desc.Usage = D3D11_USAGE_DEFAULT;
	composite_desc.ByteWidth = sizeof(CompositeConstantBuffer);
	composite_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&composite_desc, NULL, &m_pVBuffer_composite);

	// FXAA constant buffer creation
	D3D11_BUFFER_DESC fxaa_desc = {};
	fxaa_desc.Usage = D3D11_USAGE_DEFAULT;
	fxaa_desc.ByteWidth = sizeof(FxaaConstantBuffer);
	fxaa_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&fxaa_desc, NULL, &m_pVBuffer_fxaa);

	// Seleccion con clic: constant buffer del ID y textura de 1 pixel para leer el resultado en la CPU
	D3D11_BUFFER_DESC entity_id_desc = {};
	entity_id_desc.Usage = D3D11_USAGE_DEFAULT;
	entity_id_desc.ByteWidth = sizeof(EntityIdConstantBuffer);
	entity_id_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&entity_id_desc, NULL, &m_pVBuffer_entity_id);

	D3D11_TEXTURE2D_DESC staging_desc = {};
	staging_desc.Width = 1;
	staging_desc.Height = 1;
	staging_desc.MipLevels = 1;
	staging_desc.ArraySize = 1;
	staging_desc.Format = DXGI_FORMAT_R32_UINT;
	staging_desc.SampleDesc.Count = 1;
	staging_desc.Usage = D3D11_USAGE_STAGING;
	staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateTexture2D(&staging_desc, NULL, &m_entity_id_staging);

	m_gpu_profiler.init(m_engine_ptr->get_engine_props()->deviceInterface);

	UINT quality_levels_supported = 0;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT,1, &quality_levels_supported);

	/**** Depth stencil and texture creation ****/
	D3D11_TEXTURE2D_DESC depth_desc{
		.Width = win->get_window_properties()->width,
		.Height = win->get_window_properties()->height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R24G8_TYPELESS,
		.SampleDesc{
			.Count = 1,
			//.Quality = quality_levels_supported - 1,
			.Quality = 0,
		},
		.Usage = D3D11_USAGE_DEFAULT,
		//.BindFlags = D3D11_BIND_DEPTH_STENCIL 
		.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
	};
	m_depth_buffer = nullptr;
	m_depth_stencil_view = nullptr;
	
	// Depth Stencil view (for SSAO)
	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MipSlice = 0,
		},
	};

	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateTexture2D(&depth_desc, nullptr, &m_depth_buffer);				// Depth Texture
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilView(m_depth_buffer, &dsv_desc, &m_depth_stencil_view);	// Depth stencil view

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {
		.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MostDetailedMip = 0,
			.MipLevels = 1,
		},
	};

	
	//hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateShaderResourceView(m_depth_buffer, &srv_desc, &m_depth_srv);

	create_backbuffers();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &(win->get_window_info()->backbuffer), m_depth_stencil_view);	// last argument is depth stencill view

	

	// Flags table
	/*
		Flag						CPU Access	GPU Access
		D3D11_USAGE_DEFAULT			None		Read/Write
		DXD11_USAGE_IMMUTABLE		None		Read Only
		DXD11_USAGE_DYNAMIC			Write Only	Read Only
		DXD11_USAGE_STAGING			Read/Write	Read/Write
	*/

	/********/

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{
		.DepthEnable = true,
#ifdef DEPTH_PREPASS
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
#endif
#ifndef DEPTH_PREPASS
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
#endif
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,		// For multiple lights
		//.DepthFunc = D3D11_COMPARISON_LESS,			// For one light only	
	};
	m_depth_stencil_state = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_stencil_desc, &m_depth_stencil_state);	
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state,1);

	
	D3D11_DEPTH_STENCIL_DESC depth_desc_prepass = {
		.DepthEnable = true,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
		.StencilEnable = false,
	};
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_desc_prepass, &m_depth_only_state);
	
	// Depth Stencil for SSAO
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc_ssao = {
	  .DepthEnable = false,
	  .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
	  .DepthFunc = D3D11_COMPARISON_ALWAYS,
	  .StencilEnable = false,
	  .StencilReadMask = 0,
	  .StencilWriteMask = 0,
	};
	m_depth_stencil_ssao = nullptr;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_stencil_desc_ssao, &m_depth_stencil_ssao);
	

	// Blend states
	D3D11_BLEND_DESC blend_overwrite_desc{
		.AlphaToCoverageEnable = false,
		.IndependentBlendEnable = false,
		.RenderTarget = {
			{ // 0
			.BlendEnable = false,
			.SrcBlend = D3D11_BLEND_ONE,
			.DestBlend = D3D11_BLEND_ZERO,
			.BlendOp = D3D11_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D11_BLEND_ONE,
			.DestBlendAlpha = D3D11_BLEND_ZERO,
			.BlendOpAlpha = D3D11_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
		}
	  },
	};

	m_blend_state_overwrite = nullptr;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBlendState(&blend_overwrite_desc, &m_blend_state_overwrite);
	CheckShaderError(hr);

	D3D11_BLEND_DESC blend_additive_desc{
		.AlphaToCoverageEnable = false,
		.IndependentBlendEnable = false,
		.RenderTarget = {
			{ // 0
			.BlendEnable = true,
			.SrcBlend = D3D11_BLEND_ONE,
			//.SrcBlend = D3D11_BLEND_SRC_ALPHA,
			.DestBlend = D3D11_BLEND_ONE,
			//.DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
			.BlendOp = D3D11_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D11_BLEND_ZERO,			
			//.SrcBlendAlpha = D3D11_BLEND_ONE,			
			.DestBlendAlpha = D3D11_BLEND_ONE,
			//.DestBlendAlpha = D3D11_BLEND_ZERO,
			.BlendOpAlpha = D3D11_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
		}
	  },
	};
	
	m_blend_state_additive = nullptr;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBlendState(&blend_additive_desc, &m_blend_state_additive);
	CheckShaderError(hr);


	// Skybox buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(m_skybox_vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData_skybox = {};
	initData_skybox.pSysMem = m_skybox_vertices;

	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&bd, &initData_skybox, &m_pVBuffer_skybox_buffer);



	// create the input layout object for Forward rendering
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 8 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		
	};
	m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied,4,VS->GetBufferPointer(), VS->GetBufferSize(), &m_pLayout);

	// create the input layout object for Deferred rendering
	D3D11_INPUT_ELEMENT_DESC ied_deferred[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3 , D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied_deferred, 2, VS_deferred->GetBufferPointer(), VS_deferred->GetBufferSize(), &m_pLayout_deferred);
	CheckShaderError(hr);

	// Skybox input layout
	D3D11_INPUT_ELEMENT_DESC ied_skybox[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied_skybox, 1, VS_skybox->GetBufferPointer(), VS_skybox->GetBufferSize(), &m_pLayout_skybox);

	D3D11_INPUT_ELEMENT_DESC ied_depth_pass[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};

	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied_depth_pass, 1, VS_depth->GetBufferPointer(), VS_depth->GetBufferSize(), &m_pLayout_depth);
	CheckShaderError(hr);

	D3D11_INPUT_ELEMENT_DESC ied_depth_batch[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"OBJECT_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied_depth_batch, 2, VS_depth_batch->GetBufferPointer(), VS_depth_batch->GetBufferSize(), &m_pLayout_depth_batch);
	CheckShaderError(hr);

	m_sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	m_sampler_desc.MaxAnisotropy = 8;

	m_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	m_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//m_sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;	// For shadow mapping
	
	m_sampler_desc.MinLOD = 0;
	m_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_sampler_state = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateSamplerState(&m_sampler_desc, &m_sampler_state);

	// Sampler noise ssao
	D3D11_SAMPLER_DESC sampler_ssao_desc = {
		//.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.Filter = D3D11_FILTER_ANISOTROPIC,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 16,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX,
	};
	m_sampler_state_noise_ssao = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateSamplerState(&sampler_ssao_desc, &m_sampler_state_noise_ssao);

	
	D3D11_SAMPLER_DESC emissive_sampler_desc = {};
	emissive_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Bilinear filtering
	emissive_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	emissive_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	emissive_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	emissive_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	emissive_sampler_desc.MinLOD = 0;
	emissive_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_sampler_state_emissive = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateSamplerState(&emissive_sampler_desc, &m_sampler_state_emissive);

	set_draw_mode(DrawMode::Solid);

	win->m_renderer = this;
	clear_depth();

	init_ssao();
	init_reflection_probes();

	return m_isInitialized;
}

void Renderer::active_shader(ShaderType type){
	switch (type){
	case ShaderType::GeometryPass:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred, nullptr, 0);
		break;
	case ShaderType::DeferredLights:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_lights, nullptr, 0);
		break;
	case ShaderType::DeferredComposite:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_composite, nullptr, 0);
		break;
	case ShaderType::Fxaa:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_fxaa, nullptr, 0);
		break;
	case ShaderType::DirectionalLight:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_directional, nullptr, 0);
		break;
	case ShaderType::PointLight:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_point, nullptr, 0);
		break;
	case ShaderType::SpotLight:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_spot, nullptr, 0);
		break;
	case ShaderType::BloomDownsample:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_emissive_downsample, nullptr, 0);
		break;
	case ShaderType::Skybox:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_skybox, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_skybox, nullptr, 0);
		break;
	case ShaderType::EntityId:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_depth_prepass, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_entity_id, nullptr, 0);
		break;
	case ShaderType::Depth:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_depth_batch, nullptr, 0);
		// Sin pixel shader: solo se escribe el depth buffer
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(nullptr, nullptr, 0);
		break;
	case ShaderType::SSAO:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_SSAO, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_SSAO, nullptr, 0);
		break;
	case ShaderType::BlurSSAO:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_SSAO_Blur_blend, nullptr, 0);
		break;
	case ShaderType::ProbeCapture:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_probe_capture, nullptr, 0);
		break;
	case ShaderType::ProbePrefilter:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_probe_prefilter, nullptr, 0);
		break;

	default:break;
	}
}

void Renderer::render_forward(EntityComponentSystem& ecs){

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);

	clear_depth();
	clear_render_target();

	//m_engine_ptr->get_engine_props()->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_backbuffer_texture);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetConstantBuffers(0,1,&m_pVBufferConstantCamera);

	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();

	CameraConstantBuffer cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);
	cam_buffer.camera_position = m_cam->get_position();
	
	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	auto directional_light = ecs.viewComponents<DirectionalLight>();
	auto point_light = ecs.viewComponents<PointLight>();
	auto spot_light = ecs.viewComponents<SpotLight>();

	active_shader(ShaderType::DirectionalLight);
	
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);

	
	for (auto [entity, directional] : directional_light.each()) {


		if (!directional.get_enabled()) continue;

		directional.update();
		directional.upload_data();

		for (auto [entity, trans, mesh] : transforms.each()) {
			for (Mesh& m : mesh.get_model()->meshes) {
				//__debugbreak();
				render_mesh_internal(&cam_buffer, trans, m);
			}
		}
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
	}
	

	// Change blending
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);

	active_shader(ShaderType::PointLight);
	for (auto [entity, point] : point_light.each()) {

		if (!point.get_enabled()) continue;
		point.update();
		point.upload_data();

		for (auto [entity, trans, mesh] : transforms.each()) {
			for (Mesh& m : mesh.get_model()->meshes) {
				render_mesh_internal(&cam_buffer, trans, m);
			}
		}
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
	}
	
	active_shader(ShaderType::SpotLight);
	for (auto [entity, spot] : spot_light.each()) {

		if (!spot.get_enabled()) continue;
		spot.update();
		spot.upload_data();

		for (auto [entity, trans, mesh] : transforms.each()) {
			for (Mesh& m : mesh.get_model()->meshes) {
				render_mesh_internal(&cam_buffer, trans, m);
			}
		}
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
	}

	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);


	


#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float, std::milli>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(Engine::get_instance()->m_current_scene);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float, std::milli>(elapsed_imgui).count();

#endif

}

void Renderer::render_deferred(EntityComponentSystem& ecs_old){

	EntityComponentSystem& ecs = Engine::get_instance()->get_scene()->m_ecs;


#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif

	ID3D11DeviceContext* gpu_context = m_engine_ptr->get_engine_props()->inmediateDeviceContext;
	m_gpu_profiler.begin_frame(gpu_context);

	clear_render_target();
	clear_depth();
	m_gpu_profiler.mark(gpu_context, GpuPass::Clear);

	m_time += m_engine_ptr->get_delta_time();
	update_visible_meshes(ecs);
	update_reflection_probes(ecs);

#ifdef DEPTH_PREPASS
	depth_pass(ecs);
	m_gpu_profiler.mark(gpu_context, GpuPass::Depth);

	// Necesita la profundidad del prepass para que solo escriba la superficie visible
	int pick_x = 0;
	int pick_y = 0;
	if (ImguiManager::get_instance()->consume_pick_request(pick_x, pick_y)) {
		pick_entity(ecs, pick_x, pick_y);
	}
#endif // DEPTH_PREPASS

	active_shader(ShaderType::GeometryPass);

	ID3D11RenderTargetView* gbuffer_rtv[] = {
		m_deferred_resources.gbuffer_albedo_render_target_view,
		m_deferred_resources.gbuffer_normals_render_target_view,
		m_deferred_resources.gbuffer_material_render_target_view,
		m_deferred_resources.gbuffer_emissive_render_target_view,
		m_deferred_resources.gbuffer_layers_render_target_view,
	};

	auto props = Engine::get_instance()->get_engine_props();

	// Unbind render targets
	clear_shader_reources();

	// Clear render targets
	//for (int i = 0; i < ARRAYSIZE(gbuffer_rtv); ++i) props->inmediateDeviceContext->ClearRenderTargetView(gbuffer_rtv[i], m_clear_emissive_color);

	

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state, 1);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetState(m_current_raster);

	// Geometry pass
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);

	

	
	// Bind render targets
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(gbuffer_rtv), gbuffer_rtv, m_depth_stencil_view);



	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();
	const Mat4* proj_inverse = m_cam->get_projection_inverse();
	Mat4 proj_inverse_transposed = DirectX::XMMatrixTranspose(*proj_inverse);

	CameraConstantBuffer cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);
	cam_buffer.camera_position = m_cam->get_position();
	

	for (VisibleMesh& visible : m_visible_meshes) {
		render_mesh_internal(&cam_buffer, *visible.transform, *visible.mesh);
	}
	// Unbind render targets
	clear_shader_reources();

	m_gpu_profiler.mark(gpu_context, GpuPass::GBuffer);

	// SSAO Pass
	if (m_render_info_parameters.ssao_active) {
		draw_ssao(&cam_buffer.projection, &cam_buffer.view, &proj_inverse_transposed);
	}
	else {
		props->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.ssao_blur_render_target_view, m_clear_emissive_color);
	}


	// Light Pass
	//float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//props->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.postprocess_render_target_view, clear_color);


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(1, 1, &m_pVBufferDeferredConstantCamera);


	ID3D11RenderTargetView* light_pass_rtvs[] = {
		m_deferred_resources.postprocess_render_target_view, // Final color
		m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[0]	// first mipmap level
	};
	// Sin depth: este pase lo lee como textura para reconstruir la posicion, y no se puede leer y escribir a la vez
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(light_pass_rtvs), light_pass_rtvs, nullptr);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;

	update_light_buffer(ecs);

	ID3D11ShaderResourceView* srvs[] = {
		m_deferred_resources.gbuffer_albedo_shader_resource_view,
		m_deferred_resources.depth_srv,
		m_deferred_resources.gbuffer_normals_shader_resource_view,
		m_deferred_resources.gbuffer_material_shader_resource_view,
		m_deferred_resources.gbuffer_emissive_shader_resource_view,
		m_engine_ptr->m_resource.get_specular_srv(),
		m_engine_ptr->m_resource.get_brdf_srv(),
		m_light_buffer_srv,
		m_engine_ptr->m_resource.get_irradiance_srv(),
		m_render_info_parameters.ssao_active ? m_deferred_resources.ssao_blur_shader_resource_view : nullptr,
	};
	props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	ID3D11ShaderResourceView* extra_srvs[] = {
		m_deferred_resources.gbuffer_layers_shader_resource_view,
		m_probe_array_srv,
	};
	props->inmediateDeviceContext->PSSetShaderResources(10, ARRAYSIZE(extra_srvs), extra_srvs);
	props->inmediateDeviceContext->PSSetConstantBuffers(2, 1, &m_pVBuffer_probes);

	CameraDeferredConstantBuffer cam_buffer_light;
	Mat4 view_proj = (*view) * (*proj);
	cam_buffer_light.inv_view_proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, view_proj));;
	cam_buffer_light.camera_position = m_cam->get_position();
	cam_buffer_light.cubemap_max_mip_level = m_engine_ptr->m_resource.get_specular_max_miplevel();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBufferDeferredConstantCamera, 0, nullptr, &cam_buffer_light, 0, 0);


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);


	
	active_shader(ShaderType::DeferredLights);
	props->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_light_pass);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	// El blur se vuelve a usar como render target en el siguiente frame
	ID3D11ShaderResourceView* null_srvs[3] = { nullptr, nullptr, nullptr };
	props->inmediateDeviceContext->PSSetShaderResources(9, 3, null_srvs);

	m_gpu_profiler.mark(gpu_context, GpuPass::Lights);

	// Unbind light pass rtv
	//ID3D11RenderTargetView* nullRTV[] = { nullptr, nullptr };

	
	clear_shader_reources();

	draw_skybox();
	m_gpu_profiler.mark(gpu_context, GpuPass::Skybox);

	if (m_render_info_parameters.bloom_active) {
		draw_emissive_downsample();
		m_gpu_profiler.mark(gpu_context, GpuPass::Bloom);
	}

	clear_srv(2);

	// La imagen final va a una textura intermedia: el FXAA la lee para escribir en el backbuffer
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.fxaa_input_render_target_view, nullptr);

	// Setear textura resultado (SRV)
	if (m_render_info_parameters.bloom_active) {
		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[0]);
	}else {
		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &(m_deferred_resources.postprocess_resource_view));
	}

	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	// Dibujar quad: de HDR lineal a sRGB
	CompositeConstantBuffer composite_constants{};
	composite_constants.exposure = m_render_info_parameters.exposure;
	composite_constants.tonemapper = m_render_info_parameters.tonemapper;
	props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_composite, 0, nullptr, &composite_constants, 0, 0);
	props->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_composite);

	active_shader(ShaderType::DeferredComposite);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	m_gpu_profiler.mark(gpu_context, GpuPass::Composite);

	// Draw in the back buffer
	// Sin depth: el panel G-Buffer lo va a leer como textura al dibujar ImGui
	props->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, nullptr);
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.fxaa_input_shader_resource_view);
	// Bilineal y sin repetir: el FXAA lee entre pixeles y no debe coger el lado contrario de la pantalla
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

	D3D11_TEXTURE2D_DESC fxaa_input_desc;
	m_deferred_resources.fxaa_input_texture->GetDesc(&fxaa_input_desc);
	FxaaConstantBuffer fxaa_constants{};
	fxaa_constants.texel_size = { 1.0f / fxaa_input_desc.Width, 1.0f / fxaa_input_desc.Height };
	fxaa_constants.enabled = m_render_info_parameters.fxaa_active ? 1 : 0;
	props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_fxaa, 0, nullptr, &fxaa_constants, 0, 0);
	props->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_fxaa);

	active_shader(ShaderType::Fxaa);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	m_gpu_profiler.mark(gpu_context, GpuPass::Fxaa);
	m_gpu_profiler.end_frame(gpu_context);

#ifdef ENABLE_IMGUI
	memcpy(ImguiManager::get_instance()->m_gpu_ms, m_gpu_profiler.get_pass_ms(), sizeof(float) * GPU_PASS_COUNT);
#endif

	// Limpiar SRV
	ID3D11ShaderResourceView* nullSRV = nullptr;
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &nullSRV);



#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float, std::milli>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();

	//ImVec2 scene_pos = ImGui::GetWindowPos(); // posición ventana renderizado
	//ImVec2 scene_size = ImGui::GetWindowSize();

	// Llamar al gizmo justo encima de la imagen

	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(Engine::get_instance()->m_current_scene);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	ImguiManager::get_instance()->render_guizmo(m_cam/*, scene_pos, scene_size*/);
	ImguiManager::get_instance()->gbuffer_info(&m_deferred_resources);
	ImguiManager::get_instance()->show_render_parameters(&m_render_info_parameters);
	ImguiManager::get_instance()->update_picking();

	set_draw_mode(m_render_info_parameters.draw_mode);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float, std::milli>(elapsed_imgui).count();

#endif



}

void Renderer::update_light_buffer(EntityComponentSystem& ecs){

	auto props = m_engine_ptr->get_engine_props();

	m_gpu_lights.clear();

	// La direccion de la luz es local: gira con su transform y con el de sus padres
	auto world_direction = [&](entt::entity entity, Vec3 direction) {
		DirectX::XMVECTOR result = DirectX::XMLoadFloat3(&direction);
		if (TransformComponent* transform = ecs.get_component<TransformComponent>(Entity(static_cast<unsigned int>(entity), ""))) {
			result = DirectX::XMVector3TransformNormal(result, transform->get_transform());
		}
		Vec3 output;
		DirectX::XMStoreFloat3(&output, DirectX::XMVector3Normalize(result));
		return output;
	};

	for (auto [entity, light] : ecs.viewComponents<DirectionalLight>().each()) {
		if (!light.get_enabled()) continue;

		GPULight gpu{};
		gpu.type = GPULightType::Directional;
		gpu.direction = world_direction(entity, light.get_direction());
		gpu.color = light.get_color();
		gpu.intensity = light.get_intensity();
		m_gpu_lights.push_back(gpu);
	}

	for (auto [entity, light] : ecs.viewComponents<PointLight>().each()) {
		if (!light.get_enabled()) continue;

		GPULight gpu{};
		gpu.type = GPULightType::Point;
		gpu.position = light.get_position();
		gpu.color = light.get_color();
		light.update_pulse(m_time);
		gpu.intensity = light.get_current_intensity();
		gpu.range = light.get_distance();
		gpu.fall_start = light.get_fall_start();
		gpu.constant_att = light.get_constant_attenuation();
		gpu.linear_att = light.get_linear_attenuation();
		gpu.quadratic_att = light.get_quadratic_attenuation();
		gpu.source_radius = light.get_source_radius();
		gpu.source_length = light.get_source_length();
		const Vec3 axis = light.get_source_axis();
		DirectX::XMStoreFloat3(&gpu.direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&axis)));
		m_gpu_lights.push_back(gpu);
	}

	for (auto [entity, light] : ecs.viewComponents<SpotLight>().each()) {
		if (!light.get_enabled()) continue;

		Vec3 direction = light.get_direction();

		GPULight gpu{};
		gpu.type = GPULightType::Spot;
		gpu.position = light.get_position();
		gpu.direction = world_direction(entity, direction);
		gpu.color = light.get_color();
		gpu.intensity = light.get_intensity();
		gpu.range = light.get_distance();
		gpu.cos_inner = cosf(degToRad(light.get_inner_circle()));
		gpu.cos_outer = cosf(degToRad(light.get_outside_circle()));
		gpu.fall_start = 0.8f;
		gpu.constant_att = light.get_constant_attenuation();
		gpu.linear_att = light.get_linear_attenuation();
		gpu.quadratic_att = light.get_quadratic_attenuation();
		m_gpu_lights.push_back(gpu);
	}

	const unsigned int count = static_cast<unsigned int>(m_gpu_lights.size());

	// La capacidad se duplica al crecer para no recrear el buffer cada vez que aparece una luz
	if (m_light_buffer == nullptr || count > m_light_buffer_capacity) {

		unsigned int capacity = std::max(m_light_buffer_capacity, 64u);
		while (capacity < count) capacity *= 2;

		if (m_light_buffer_srv) m_light_buffer_srv->Release();
		if (m_light_buffer) m_light_buffer->Release();
		m_light_buffer_srv = nullptr;
		m_light_buffer = nullptr;

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = capacity * sizeof(GPULight);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(GPULight);
		HRESULT hr = props->deviceInterface->CreateBuffer(&desc, nullptr, &m_light_buffer);
		assert(SUCCEEDED(hr) && "Failed to create light buffer");

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = capacity;
		hr = props->deviceInterface->CreateShaderResourceView(m_light_buffer, &srv_desc, &m_light_buffer_srv);
		assert(SUCCEEDED(hr) && "Failed to create light buffer SRV");

		m_light_buffer_capacity = capacity;
	}

	if (count > 0) {
		D3D11_MAPPED_SUBRESOURCE mapped;
		if (SUCCEEDED(props->inmediateDeviceContext->Map(m_light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
			memcpy(mapped.pData, m_gpu_lights.data(), count * sizeof(GPULight));
			props->inmediateDeviceContext->Unmap(m_light_buffer, 0);
		}
	}

	LightPassConstantBuffer constants{};
	constants.light_count = count;
	constants.ambient_intensity = m_render_info_parameters.ambient_intensity;
	constants.ssao_power = m_render_info_parameters.ssao_active ? m_render_info_parameters.ssao_intensity : 0.0f;
	constants.ssao_direct = m_render_info_parameters.ssao_direct;
	props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_light_pass, 0, nullptr, &constants, 0, 0);
}

void Renderer::set_draw_mode(DrawMode mode){

	switch (mode) {
		case DrawMode::Solid: m_current_raster = m_engine_ptr->m_raster_state; break;
		case DrawMode::Wireframe: m_current_raster = m_engine_ptr->m_raster_state_wireframe; break;
	}

	m_current_draw_mode = mode;
	ImguiManager::get_instance()->m_current_draw_mode = mode;
}

void Renderer::set_camera(CameraComponent* cam){
	assert(cam != nullptr);

	m_cam = cam;
}

DrawMode Renderer::get_current_draw_mode(){

	return m_current_draw_mode;
}

void Renderer::release(){

	const auto free_shader = [](auto*& shader) {
		if (shader) shader->Release();
		shader = nullptr;
	};

	free_shader(m_shader_files.VS_common);
	free_shader(m_shader_files.PS_directional);
	free_shader(m_shader_files.PS_point);
	free_shader(m_shader_files.PS_spot);
	free_shader(m_shader_files.VS_deferred);
	free_shader(m_shader_files.PS_deferred);
	free_shader(m_shader_files.VS_deferred_common);
	free_shader(m_shader_files.PS_deferred_composite);
	free_shader(m_shader_files.PS_deferred_fxaa);
	free_shader(m_shader_files.PS_deferred_lights);
	free_shader(m_shader_files.PS_deferred_emissive_downsample);
	free_shader(m_shader_files.VS_deferred_skybox);
	free_shader(m_shader_files.PS_deferred_skybox);
	free_shader(m_shader_files.VS_depth_prepass);
	free_shader(m_shader_files.VS_depth_batch);
	free_shader(m_shader_files.PS_entity_id);
	free_shader(m_shader_files.PS_SSAO);
	free_shader(m_shader_files.VS_SSAO);
	free_shader(m_shader_files.PS_SSAO_Blur_blend);
	free_shader(m_shader_files.PS_probe_capture);
	free_shader(m_shader_files.PS_probe_prefilter);
	release_reflection_probes();
}

void Renderer::render_mesh_internal(CameraConstantBuffer* camera_buffer, TransformComponent& trans, Mesh& m){
	
	// Set camera values to constant buffer
	camera_buffer->model = DirectX::XMMatrixTranspose(trans.get_transform());

	// La normal usa la inversa transpuesta de model. Como las matrices se suben traspuestas,
	// lo que se sube es la inversa tal cual: las dos trasposiciones se cancelan.
	camera_buffer->normal_matrix = trans.get_inverse_transform();
	camera_buffer->uv_scale = m.material.get_uv_scale();

	// Upload metallic and roughness values to constant buffer
	camera_buffer->metallic = m.material.get_metallic_value();
	camera_buffer->roughness = m.material.get_roughness_value();
	camera_buffer->color = m.material.get_color_value();
	camera_buffer->emissive = m.material.get_emissive_value();
	m.material.update_emissive_pulse(m_time);
	camera_buffer->emissive_intensity = m.material.get_current_emissive_intensity();
	camera_buffer->emissive_bloom = m.material.get_emissive_bloom();
	camera_buffer->reflectance = m.material.get_reflectance();
	camera_buffer->layers = m.material.get_layers();
	
	// Buffer de la camara con la model del objeto
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBufferConstantCamera, 0, nullptr, camera_buffer, 0, 0);

	
	// Set Vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m.buffer, &stride, &offset);

	// Upload textures
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(0, 1, &(m.material.get_albedo()->m_data.texture_view));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(1, 1, &(m.material.get_normal()->m_data.texture_view));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(2, 1, &(m.material.get_metallic()->m_data.texture_view));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(3, 1, &(m.material.get_roughness()->m_data.texture_view));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(4, 1, &(m.material.get_ao()->m_data.texture_view));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(5, 1, &(m.material.get_emissive()->m_data.texture_view));

	// Draw
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetIndexBuffer(m.index_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->DrawIndexed(m.num_indices, 0, 0);
	add_draw_call();
}

void Renderer::render_mesh_depth_only(CameraDepthPrePass* camera_buffer, TransformComponent& trans, Mesh& m){
	// Set camera values to constant buffer
	camera_buffer->model = DirectX::XMMatrixTranspose(trans.get_transform());

	// Buffer de la camara con la model del objeto
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_constant_camera_depth_only, 0, nullptr, camera_buffer, 0, 0);


	// Set Vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m.buffer, &stride, &offset);

	// Draw
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetIndexBuffer(m.index_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->DrawIndexed(m.num_indices, 0, 0);
	add_draw_call();
}

void Renderer::draw_emissive_downsample(){


	auto props = m_engine_ptr->get_engine_props();

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);

	// DownScale
	for (int i = 1; i < NUM_MIPMAPS_EMISSIVE; i++) {
		
		clear_srv(1);
		clear_rtv(1);
		active_shader(ShaderType::BloomDownsample);


		D3D11_TEXTURE2D_DESC desc;
		m_deferred_resources.gbuffer_emissive_mipmap_texture[i - 1]->GetDesc(&desc);

		set_viewport(desc.Width, desc.Height);


		props->inmediateDeviceContext->PSSetShaderResources(0,1, &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[i - 1]);		// This texture has the emissive texture (and brightness)
		props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.emissive_dowscaling_render_target_view[i - 1], nullptr);			// Disable dpeth stencil, not needed here
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		// Horizontal blur
		EmissiveConstantBuffer emissive_buffer{};
		emissive_buffer.texel_size = {1.0f / desc.Width, 1.0f / desc.Height};
		emissive_buffer.bloom_intensity = 1.0f;
		emissive_buffer.horizontal = 1;
		emissive_buffer.blend = 0;
		props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);

		UINT stride = sizeof(VertexQuad);
		UINT offset = 0;

		props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
		props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
		props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();

		clear_srv(1);
		clear_rtv(1);

		m_deferred_resources.gbuffer_emissive_mipmap_texture[i]->GetDesc(&desc);
		set_viewport(desc.Width, desc.Height);

		// Vertical blur
		EmissiveConstantBuffer emissive_buffer2{};
		emissive_buffer2.texel_size = { 1.0f / desc.Width, 1.0f / desc.Height };
		emissive_buffer2.bloom_intensity = 1.0f;
		emissive_buffer2.horizontal = 0;
		emissive_buffer2.blend = 0;
		props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer2, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);

		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.emissive_dowscaling_shader_resource_view[i - 1]);
		props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], nullptr);
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();
	}

	// UpScale
	for (int i = (NUM_MIPMAPS_EMISSIVE - 2); i >= 0; i--) {

		clear_srv(1);
		clear_rtv(1);
		active_shader(ShaderType::BloomDownsample);


		D3D11_TEXTURE2D_DESC desc;
		m_deferred_resources.gbuffer_emissive_mipmap_texture[i + 1]->GetDesc(&desc);

		set_viewport(desc.Width, desc.Height);


		props->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[i + 1]);		// This texture has the emissive texture (and brightness)
		props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.emissive_dowscaling_render_target_view[i + 1], nullptr);			// Disable dpeth stencil, not needed here
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		// Horizontal blur
		EmissiveConstantBuffer emissive_buffer{};
		emissive_buffer.texel_size = { 1.0f / desc.Width, 1.0f / desc.Height };
		emissive_buffer.bloom_intensity = 1.0f;
		emissive_buffer.horizontal = 1;
		emissive_buffer.blend = 0;
		props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);

		UINT stride = sizeof(VertexQuad);
		UINT offset = 0;

		props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
		props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
		props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();

		clear_srv(1);
		clear_rtv(1);

		m_deferred_resources.gbuffer_emissive_mipmap_texture[i]->GetDesc(&desc);
		set_viewport(desc.Width, desc.Height);

		// Vertical blur
		EmissiveConstantBuffer emissive_buffer2{};
		emissive_buffer2.texel_size = { 1.0f / desc.Width, 1.0f / desc.Height };
		emissive_buffer2.bloom_intensity = 1.0f;
		emissive_buffer2.horizontal = 0;
		emissive_buffer2.blend = 0;

		// Last iteration, blend with light
		if (i == 0) {
			ID3D11ShaderResourceView* srvs[] = {
				m_deferred_resources.emissive_dowscaling_shader_resource_view[i + 1],
				m_deferred_resources.postprocess_resource_view
			};

			props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
			emissive_buffer2.blend = 1;
			emissive_buffer2.bloom_intensity = 1.0f / (NUM_MIPMAPS_EMISSIVE - 1);	// Media de los niveles sumados
		}else {
			props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.emissive_dowscaling_shader_resource_view[i + 1]);
			// Se suma a lo que este nivel guardo al bajar: asi se conserva el brillo ajustado de los niveles grandes
			props->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
		}

		props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer2, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);

		props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], nullptr);
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();
	}

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetViewports(1, &m_engine_ptr->m_viewport);
}
  
void Renderer::draw_ssao(Mat4* projection, Mat4* view, Mat4* projection_inverse){

	auto props = m_engine_ptr->get_instance()->get_engine_props();

	// Al cambiar la opcion en el panel, las texturas se rehacen con el tamano nuevo
	if (m_render_info_parameters.ssao_half_res != m_ssao_half_built) create_ssao_targets();

	D3D11_TEXTURE2D_DESC ssao_desc;
	m_deferred_resources.ssao_texture->GetDesc(&ssao_desc);
	set_viewport(ssao_desc.Width, ssao_desc.Height);

	active_shader(ShaderType::SSAO);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetState(m_engine_ptr->m_raster_state);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_ssao, 0);
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state, 1);
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(nullptr, 0);
	props->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);

	clear_rtv(5);
	clear_srv(5);
	props->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.ssao_render_target_view, m_clear_emissive_color);
	props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.ssao_render_target_view, nullptr);

	// Position, Normals, SSAO Noise
	ID3D11ShaderResourceView* gbuffer_srv[] = {
		m_deferred_resources.gbuffer_normals_shader_resource_view,
		m_noise_srv,
		m_deferred_resources.depth_srv,
	};

	props->inmediateDeviceContext->PSSetShaderResources(0,ARRAYSIZE(gbuffer_srv), gbuffer_srv);


	// SSAO kernel constant buffer initialized one time only, not need to update every frame, just projection matrix
	m_constant_buffer_ssao.projection = *projection;
	m_constant_buffer_ssao.view = *view;
	m_constant_buffer_ssao.inv_projection = *projection_inverse;

	m_constant_buffer_ssao.samples = m_render_info_parameters.ssao_samples;
	m_constant_buffer_ssao.samples_float = static_cast<float>(m_render_info_parameters.ssao_samples);
	m_constant_buffer_ssao.ssao_base_radius = m_render_info_parameters.ssao_base_radius;
	m_constant_buffer_ssao.max_radius_px = m_render_info_parameters.ssao_max_radius_px;
	m_constant_buffer_ssao.resolution_scale = m_ssao_half_built ? 2.0f : 1.0f;
	m_constant_buffer_ssao.inv_projection = *projection_inverse;
	props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_ssao, 0, nullptr, &m_constant_buffer_ssao, 0, 0);
	props->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_ssao);

	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);
	props->inmediateDeviceContext->PSSetSamplers(1, 1, &m_sampler_state_noise_ssao);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;

	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);


	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	clear_rtv(5);
	clear_srv(5);

	m_gpu_profiler.mark(props->inmediateDeviceContext, GpuPass::SSAO);

	draw_ssao_blur();
	m_gpu_profiler.mark(props->inmediateDeviceContext, GpuPass::SSAOBlur);

	props->inmediateDeviceContext->RSSetViewports(1, &m_engine_ptr->m_viewport);

}

void Renderer::draw_ssao_blur(){
	auto props = m_engine_ptr->get_engine_props();

	active_shader(ShaderType::BlurSSAO);
	props->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.ssao_blur_render_target_view, nullptr);
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.ssao_shader_resource_view);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	clear_srv(1);
	clear_rtv(1);
}

void Renderer::update_visible_meshes(EntityComponentSystem& ecs){

	using namespace DirectX;

	m_visible_meshes.clear();
	m_scene_meshes.clear();
	m_depth_all_meshes.clear();
	m_depth_matrices.clear();
	unsigned int total = 0;
	unsigned long long visible_triangles = 0;
	unsigned long long total_triangles = 0;
	size_t key = 14695981039346656037ull;

	// Los 6 planos del frustum, en mundo. Las filas de la traspuesta son las columnas de view * proj:
	// en DirectX el vector multiplica por la izquierda. La Z de clip va de 0 a 1, por eso cerca es solo r[2].
	const Mat4 clip = XMMatrixTranspose((*m_cam->get_view()) * (*m_cam->get_projection()));
	const FVector planes[6] = {
		XMPlaneNormalize(XMVectorAdd(clip.r[3], clip.r[0])),		// Izquierda
		XMPlaneNormalize(XMVectorSubtract(clip.r[3], clip.r[0])),	// Derecha
		XMPlaneNormalize(XMVectorAdd(clip.r[3], clip.r[1])),		// Abajo
		XMPlaneNormalize(XMVectorSubtract(clip.r[3], clip.r[1])),	// Arriba
		XMPlaneNormalize(clip.r[2]),								// Cerca
		XMPlaneNormalize(XMVectorSubtract(clip.r[3], clip.r[2])),	// Lejos
	};

	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	for (auto [entity, trans, mesh] : transforms.each()) {

		const Mat4 world = trans.get_transform();

		// La esfera crece con la mayor escala de mundo, que ya incluye la de los padres
		const float scale_x = XMVectorGetX(XMVector3LengthSq(world.r[0]));
		const float scale_y = XMVectorGetX(XMVector3LengthSq(world.r[1]));
		const float scale_z = XMVectorGetX(XMVector3LengthSq(world.r[2]));
		const float max_scale = sqrtf(std::max(scale_x, std::max(scale_y, scale_z)));

		XMFLOAT4X4 world_transposed;
		XMStoreFloat4x4(&world_transposed, XMMatrixTranspose(world));

		for (Mesh& m : mesh.get_model()->meshes) {
			const unsigned int slot = total++;
			total_triangles += m.num_indices / 3;
			m_depth_all_meshes.push_back(&m);
			m_depth_matrices.push_back(world_transposed);
			key = (key ^ reinterpret_cast<size_t>(m.buffer)) * 1099511628211ull;
			key = (key ^ m.vertices.size()) * 1099511628211ull;

			if (mesh.is_visible_in_probes()) m_scene_meshes.push_back({ &trans, &m, slot });

			bool visible = true;
			if (m_render_info_parameters.frustum_culling && m.bounds_radius >= 0.0f) {
				const FVector center = XMVector3TransformCoord(XMLoadFloat3(&m.bounds_center), world);
				const float radius = m.bounds_radius * max_scale;
				for (const FVector& plane : planes) {
					if (XMVectorGetX(XMPlaneDotCoord(plane, center)) < -radius) {
						visible = false;
						break;
					}
				}
			}
			if (visible) {
				m_visible_meshes.push_back({ &trans, &m, slot });
				visible_triangles += m.num_indices / 3;
			}
		}
	}
	m_depth_geometry_key = key;

#ifdef ENABLE_IMGUI
	ImguiManager::get_instance()->m_visible_meshes = static_cast<unsigned int>(m_visible_meshes.size());
	ImguiManager::get_instance()->m_total_meshes = total;
	ImguiManager::get_instance()->m_visible_triangles = visible_triangles;
	ImguiManager::get_instance()->m_total_triangles = total_triangles;
#endif
}

void Renderer::rebuild_depth_geometry(){

	auto device = m_engine_ptr->get_engine_props()->deviceInterface;

	if (m_depth_vertex_buffer) m_depth_vertex_buffer->Release();
	if (m_depth_index_buffer) m_depth_index_buffer->Release();
	if (m_depth_matrix_buffer) m_depth_matrix_buffer->Release();
	if (m_depth_matrix_srv) m_depth_matrix_srv->Release();
	m_depth_vertex_buffer = nullptr;
	m_depth_index_buffer = nullptr;
	m_depth_matrix_buffer = nullptr;
	m_depth_matrix_srv = nullptr;
	m_depth_uploaded_slots.clear();
	m_depth_index_count = 0;
	m_depth_built_key = m_depth_geometry_key;

	std::vector<DepthVertex> vertices;
	m_depth_indices.clear();
	m_depth_ranges.clear();

	for (unsigned int slot = 0; slot < m_depth_all_meshes.size(); slot++) {
		const Mesh& m = *m_depth_all_meshes[slot];
		const unsigned int base_vertex = static_cast<unsigned int>(vertices.size());

		for (const Vertex& v : m.vertices) {
			vertices.push_back({ v.pos_x, v.pos_y, v.pos_z, slot });
		}

		m_depth_ranges.push_back({ static_cast<unsigned int>(m_depth_indices.size()), static_cast<unsigned int>(m.indices.size()) });
		for (unsigned int index : m.indices) {
			m_depth_indices.push_back(base_vertex + index);
		}
	}

	if (vertices.empty() || m_depth_indices.empty()) return;

	D3D11_BUFFER_DESC vertex_desc = {};
	vertex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vertex_desc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(DepthVertex));
	vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertex_data = {};
	vertex_data.pSysMem = vertices.data();
	device->CreateBuffer(&vertex_desc, &vertex_data, &m_depth_vertex_buffer);

	D3D11_BUFFER_DESC index_desc = {};
	index_desc.Usage = D3D11_USAGE_DYNAMIC;
	index_desc.ByteWidth = static_cast<UINT>(m_depth_indices.size() * sizeof(unsigned int));
	index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&index_desc, nullptr, &m_depth_index_buffer);

	D3D11_BUFFER_DESC matrix_desc = {};
	matrix_desc.Usage = D3D11_USAGE_DYNAMIC;
	matrix_desc.ByteWidth = static_cast<UINT>(m_depth_all_meshes.size() * sizeof(DirectX::XMFLOAT4X4));
	matrix_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	matrix_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrix_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	matrix_desc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
	device->CreateBuffer(&matrix_desc, nullptr, &m_depth_matrix_buffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.NumElements = static_cast<UINT>(m_depth_all_meshes.size());
	device->CreateShaderResourceView(m_depth_matrix_buffer, &srv_desc, &m_depth_matrix_srv);
}

void Renderer::depth_pass(EntityComponentSystem& ecs){
	
	active_shader(ShaderType::Depth);
	auto props = m_engine_ptr->get_engine_props();
	props->inmediateDeviceContext->OMSetRenderTargets(0, nullptr, m_depth_stencil_view);
	props->inmediateDeviceContext->OMSetDepthStencilState(m_depth_only_state, 1);
	props->inmediateDeviceContext->RSSetState(m_current_raster);

	props->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBuffer_constant_camera_depth_only);
	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();

	CameraDepthPrePass cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);

	props->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_constant_camera_depth_only, 0, nullptr, &cam_buffer, 0, 0);

	if (m_depth_geometry_key != m_depth_built_key) rebuild_depth_geometry();
	if (!m_depth_vertex_buffer || m_depth_all_meshes.empty()) return;

	// Indices: solo si ha cambiado lo que se ve. Son copias de rangos, sin recorrer cada indice.
	bool same_slots = m_depth_uploaded_slots.size() == m_visible_meshes.size();
	for (size_t i = 0; same_slots && i < m_visible_meshes.size(); i++) {
		same_slots = m_depth_uploaded_slots[i] == m_visible_meshes[i].slot;
	}
	if (!same_slots) {
		D3D11_MAPPED_SUBRESOURCE mapped;
		if (SUCCEEDED(props->inmediateDeviceContext->Map(m_depth_index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
			unsigned int* destination = static_cast<unsigned int*>(mapped.pData);
			m_depth_index_count = 0;
			m_depth_uploaded_slots.clear();
			for (const VisibleMesh& visible : m_visible_meshes) {
				const DepthRange& range = m_depth_ranges[visible.slot];
				if (range.index_count > 0) memcpy(destination + m_depth_index_count, m_depth_indices.data() + range.first_index, range.index_count * sizeof(unsigned int));
				m_depth_index_count += range.index_count;
				m_depth_uploaded_slots.push_back(visible.slot);
			}
			props->inmediateDeviceContext->Unmap(m_depth_index_buffer, 0);
		}
	}
	if (m_depth_index_count == 0) return;

	D3D11_MAPPED_SUBRESOURCE mapped_matrices;
	if (FAILED(props->inmediateDeviceContext->Map(m_depth_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_matrices))) return;
	memcpy(mapped_matrices.pData, m_depth_matrices.data(), m_depth_matrices.size() * sizeof(DirectX::XMFLOAT4X4));
	props->inmediateDeviceContext->Unmap(m_depth_matrix_buffer, 0);

	UINT stride = sizeof(DepthVertex);
	UINT offset = 0;
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_depth_batch);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_depth_vertex_buffer, &stride, &offset);
	props->inmediateDeviceContext->IASetIndexBuffer(m_depth_index_buffer, DXGI_FORMAT_R32_UINT, 0);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->VSSetShaderResources(0, 1, &m_depth_matrix_srv);

	props->inmediateDeviceContext->DrawIndexed(m_depth_index_count, 0, 0);
	add_draw_call();

	ID3D11ShaderResourceView* null_srv = nullptr;
	props->inmediateDeviceContext->VSSetShaderResources(0, 1, &null_srv);

	//clear_rtv(1);
}

void Renderer::pick_entity(EntityComponentSystem& ecs, int x, int y){

	ID3D11DeviceContext* context = m_engine_ptr->get_engine_props()->inmediateDeviceContext;

	D3D11_TEXTURE2D_DESC id_desc;
	m_deferred_resources.entity_id_texture->GetDesc(&id_desc);
	if (x < 0 || y < 0 || x >= static_cast<int>(id_desc.Width) || y >= static_cast<int>(id_desc.Height)) return;

	// 0 = ningun objeto; por eso cada ID se guarda sumando 1
	const float clear_id[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(m_deferred_resources.entity_id_rtv, clear_id);

	active_shader(ShaderType::EntityId);
	context->OMSetRenderTargets(1, &m_deferred_resources.entity_id_rtv, m_depth_stencil_view);
	context->OMSetDepthStencilState(m_depth_stencil_state, 1);
	context->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	context->RSSetState(m_engine_ptr->m_raster_state);
	context->IASetInputLayout(m_pLayout_depth);
	context->VSSetConstantBuffers(0, 1, &m_pVBuffer_constant_camera_depth_only);
	context->PSSetConstantBuffers(0, 1, &m_pVBuffer_entity_id);

	CameraDepthPrePass cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*m_cam->get_view());
	cam_buffer.projection = DirectX::XMMatrixTranspose(*m_cam->get_projection());

	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	for (auto [entity, trans, mesh] : transforms.each()) {

		EntityIdConstantBuffer id_constants{};
		id_constants.entity_id = static_cast<uint32>(entity) + 1;
		context->UpdateSubresource(m_pVBuffer_entity_id, 0, nullptr, &id_constants, 0, 0);

		for (Mesh& m : mesh.get_model()->meshes) {
			render_mesh_depth_only(&cam_buffer, trans, m);
		}
	}

	clear_rtv(1);

	// Se copia solo el pixel del clic y se espera a la GPU: un instante, y solo en este frame
	const D3D11_BOX pixel = { static_cast<UINT>(x), static_cast<UINT>(y), 0, static_cast<UINT>(x) + 1, static_cast<UINT>(y) + 1, 1 };
	context->CopySubresourceRegion(m_entity_id_staging, 0, 0, 0, 0, m_deferred_resources.entity_id_texture, 0, &pixel);

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (FAILED(context->Map(m_entity_id_staging, 0, D3D11_MAP_READ, 0, &mapped))) return;
	const uint32 picked = *static_cast<const uint32*>(mapped.pData);
	context->Unmap(m_entity_id_staging, 0);

	Scene* scene = m_engine_ptr->get_scene();
	if (picked != 0) {
		const unsigned int picked_id = picked - 1;
		for (Entity& e : ecs.get_entities()) {
			if (e.get_id() == picked_id) {
				scene->select_entity(e);
				return;
			}
		}
	}
	scene->select_entity(Entity(static_cast<unsigned int>(-1), ""));
}

void Renderer::draw_skybox(){
	draw_skybox_to(m_deferred_resources.postprocess_render_target_view, m_depth_stencil_view, *m_cam->get_view(), *m_cam->get_projection(), m_cam->get_position());
}

void Renderer::draw_skybox_to(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, const Mat4& view, const Mat4& proj, const Vec3& position){
	active_shader(ShaderType::Skybox);
	//clear_depth();

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetState(m_engine_ptr->m_raster_state_skybox);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state, 1);

	auto props = m_engine_ptr->get_engine_props();
	props->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);

	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_skybox);
	props->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);

	ID3D11ShaderResourceView* srv = m_engine_ptr->m_resource.get_skybox_srv();

	//props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.gbuffer_albedo_render_target_view, nullptr);
	props->inmediateDeviceContext->OMSetRenderTargets(1, &rtv, dsv);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	CameraConstantBuffer cam_buffer{};
	cam_buffer.view = DirectX::XMMatrixTranspose(view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(proj);
	cam_buffer.camera_position = position;
	props->inmediateDeviceContext->UpdateSubresource(m_pVBufferConstantCamera, 0, nullptr, &cam_buffer, 0, 0);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_skybox_buffer, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &srv);
	props->inmediateDeviceContext->Draw(36,0);
	add_draw_call();
}

void Renderer::clear_shader_reources(int size){

	clear_rtv(size);
	clear_srv(size);
}

void Renderer::create_backbuffers(){
	Engine* e = Engine::get_instance();

	RECT clientRect;
	GetClientRect(m_window->get_window_info()->window_handle, &clientRect);
	UINT width_w = clientRect.right - clientRect.left;
	UINT height_w = clientRect.bottom - clientRect.top;

	width_w &= ~1;
	height_w &= ~1;


	if (m_window->get_window_info()->backbuffer) {
		m_window->get_window_info()->backbuffer->Release();
		m_window->get_window_info()->backbuffer = nullptr;
	}

	if (m_backbuffer_texture) {
		m_backbuffer_texture->Release();
		m_backbuffer_texture = nullptr;
	}
	
	HRESULT hr = e->get_engine_props()->swapChain->ResizeBuffers(NUM_BUFFERING, width_w, height_w, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	// Backbuffer

	ID3D11Texture2D* backBuffer = nullptr;
	hr = e->get_engine_props()->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr)) {
		printf("GetBuffer(%d) failed with HRESULT 0x%08X\n", 0, hr);
		assert(false && "Check console for more information");
	}


	D3D11_TEXTURE2D_DESC desc;
	backBuffer->GetDesc(&desc);
	printf("Backbuffer size: %u x %u\n", desc.Width, desc.Height);

	e->get_engine_props()->deviceInterface->CreateRenderTargetView(backBuffer, nullptr, &m_window->get_window_info()->backbuffer);
	m_backbuffer_texture = backBuffer;
	//backBuffer->Release();
	
}

void Renderer::create_ssao_targets(){

	auto device = Engine::get_instance()->get_engine_props()->deviceInterface;
	const bool half = m_render_info_parameters.ssao_half_res;
	const unsigned int width = half ? (m_target_width + 1) / 2 : m_target_width;
	const unsigned int height = half ? (m_target_height + 1) / 2 : m_target_height;

	// R: oclusion. G: distancia a la camara, para que el pase de luces sepa de que superficie viene cada valor
	auto create = [&](ID3D11Texture2D** texture, ID3D11RenderTargetView** rtv, ID3D11ShaderResourceView** srv) {
		if (*texture) (*texture)->Release();
		if (*rtv) (*rtv)->Release();
		if (*srv) (*srv)->Release();

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		device->CreateTexture2D(&desc, nullptr, texture);
		device->CreateRenderTargetView(*texture, nullptr, rtv);
		device->CreateShaderResourceView(*texture, nullptr, srv);
	};
	create(&m_deferred_resources.ssao_texture, &m_deferred_resources.ssao_render_target_view, &m_deferred_resources.ssao_shader_resource_view);
	create(&m_deferred_resources.ssao_blur_texture, &m_deferred_resources.ssao_blur_render_target_view, &m_deferred_resources.ssao_blur_shader_resource_view);
	m_ssao_half_built = half;
}

void Renderer::create_deferred_resources(unsigned int width, unsigned int height){
	Engine* e = Engine::get_instance();
	auto device = e->get_engine_props()->deviceInterface;
	m_target_width = width;
	m_target_height = height;

	auto create_render_target = [&](DXGI_FORMAT format, ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRTV, ID3D11ShaderResourceView** outSRV, unsigned int width_tmp = 0, unsigned int height_tmp = 0) {
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width_tmp == 0 ? width : width_tmp;
		desc.Height = height_tmp == 0 ? height: height_tmp;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		HRESULT hr = device->CreateTexture2D(&desc, nullptr, outTex);
		assert(SUCCEEDED(hr) && "Failed to create G-Buffer texture");

		hr = device->CreateRenderTargetView(*outTex, nullptr, outRTV);
		assert(SUCCEEDED(hr) && "Failed to create G-Buffer RTV");

		hr = device->CreateShaderResourceView(*outTex, nullptr, outSRV);
		assert(SUCCEEDED(hr) && "Failed to create G-Buffer SRV");
	};

	// Albedo: sRGB para no perder precision en los oscuros con 8 bits (la GPU convierte al leer y escribir)
	create_render_target(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		&m_deferred_resources.gbuffer_albedo_texture,
		&m_deferred_resources.gbuffer_albedo_render_target_view,
		&m_deferred_resources.gbuffer_albedo_shader_resource_view
	);
	
	// Normals (use a high-precision format for better quality)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_normals_texture,
		&m_deferred_resources.gbuffer_normals_render_target_view,
		&m_deferred_resources.gbuffer_normals_shader_resource_view
	);

	// Material info (Metallic, Roughness, AO, Specular � pack into 4 channels)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_material_texture,
		&m_deferred_resources.gbuffer_material_render_target_view,
		&m_deferred_resources.gbuffer_material_shader_resource_view
	);

	// Emissive (Input)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_emissive_texture,
		&m_deferred_resources.gbuffer_emissive_render_target_view,
		&m_deferred_resources.gbuffer_emissive_shader_resource_view
	);

	// Capas del material. RG: normal de la malla (octaedro), para el barniz. B: clear coat y su rugosidad.
	// A: sheen y su tinte. B y A llevan dos valores de 8 bits cada uno.
	create_render_target(DXGI_FORMAT_R16G16B16A16_UNORM,
		&m_deferred_resources.gbuffer_layers_texture,
		&m_deferred_resources.gbuffer_layers_render_target_view,
		&m_deferred_resources.gbuffer_layers_shader_resource_view
	);
	
	// Emissive Mipmaps
	for (unsigned int i = 0; i < NUM_MIPMAPS_EMISSIVE; i++) {

		unsigned int mip_width = std::max(1u, width >> (i));
		unsigned int mip_height= std::max(1u, height >> (i));

		create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
			&m_deferred_resources.gbuffer_emissive_mipmap_texture[i],
			&m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i],
			&m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[i],
			mip_width,
			mip_height
		);
		
		create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
			&m_deferred_resources.emissive_dowscaling_texture[i],
			&m_deferred_resources.emissive_dowscaling_render_target_view[i],
			&m_deferred_resources.emissive_dowscaling_shader_resource_view[i],
			mip_width,
			mip_height
		);


	}

	// PostProcess
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.postprocess_texture,
		&m_deferred_resources.postprocess_render_target_view,
		&m_deferred_resources.postprocess_resource_view
	);

	// Entrada del FXAA: guarda los valores ya en sRGB, por eso el formato no es _SRGB
	create_render_target(DXGI_FORMAT_R8G8B8A8_UNORM,
		&m_deferred_resources.fxaa_input_texture,
		&m_deferred_resources.fxaa_input_render_target_view,
		&m_deferred_resources.fxaa_input_shader_resource_view
	);

	// SSAO y su blur, a resolucion completa o a la mitad
	create_ssao_targets();
	
	// IDs de entidad, para seleccionar con clic
	create_render_target(DXGI_FORMAT_R32_UINT,
		&m_deferred_resources.entity_id_texture,
		&m_deferred_resources.entity_id_rtv,
		&m_deferred_resources.entity_id_srv
	);


}

void Renderer::release_deferred_resources(){

	auto func = [&](ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRTV, ID3D11ShaderResourceView** outSRV) {

		if (*outTex) (*outTex)->Release(); (*outTex) = nullptr;
		if (*outRTV) (*outRTV)->Release(); (*outRTV) = nullptr;
		if (*outSRV) (*outSRV)->Release(); (*outSRV) = nullptr;
	};

	func(&m_deferred_resources.gbuffer_albedo_texture, &m_deferred_resources.gbuffer_albedo_render_target_view, &m_deferred_resources.gbuffer_albedo_shader_resource_view);
	func(&m_deferred_resources.gbuffer_normals_texture, &m_deferred_resources.gbuffer_normals_render_target_view, &m_deferred_resources.gbuffer_normals_shader_resource_view);
	func(&m_deferred_resources.gbuffer_material_texture, &m_deferred_resources.gbuffer_material_render_target_view, &m_deferred_resources.gbuffer_material_shader_resource_view);
	func(&m_deferred_resources.gbuffer_emissive_texture, &m_deferred_resources.gbuffer_emissive_render_target_view, &m_deferred_resources.gbuffer_emissive_shader_resource_view);
	func(&m_deferred_resources.gbuffer_layers_texture, &m_deferred_resources.gbuffer_layers_render_target_view, &m_deferred_resources.gbuffer_layers_shader_resource_view);
	
	for (unsigned int i = 0; i < NUM_MIPMAPS_EMISSIVE; i++) {
		func(&m_deferred_resources.gbuffer_emissive_mipmap_texture[i], &m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[i]);
		func(&m_deferred_resources.emissive_dowscaling_texture[i], &m_deferred_resources.emissive_dowscaling_render_target_view[i], &m_deferred_resources.emissive_dowscaling_shader_resource_view[i]);
	}

	func(&m_deferred_resources.postprocess_texture, &m_deferred_resources.postprocess_render_target_view,&m_deferred_resources.postprocess_resource_view);
	func(&m_deferred_resources.fxaa_input_texture, &m_deferred_resources.fxaa_input_render_target_view, &m_deferred_resources.fxaa_input_shader_resource_view);
	func(&m_deferred_resources.ssao_texture, &m_deferred_resources.ssao_render_target_view,&m_deferred_resources.ssao_shader_resource_view);
	func(&m_deferred_resources.ssao_blur_texture, &m_deferred_resources.ssao_blur_render_target_view, &m_deferred_resources.ssao_blur_shader_resource_view);
	func(&m_deferred_resources.entity_id_texture, &m_deferred_resources.entity_id_rtv, &m_deferred_resources.entity_id_srv);
}

void Renderer::init_ssao(){

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	ZeroMemory(&m_constant_buffer_ssao, sizeof(m_constant_buffer_ssao));

	//Kernel
	for (unsigned int i = 0; i < ssao_kernel; i++) {


		FVector random({ (dist(mt) * 2.0f) - 1.0f, (dist(mt) * 2.0f) - 1.0f, dist(mt) }); // Tangent space z positive to rotate around z
		//FVector random({ /*(*/dist(mt)/* * 2.0f) - 1.0f*/, /*(*/dist(mt)/* * 2.0f) - 1.0f*/, 0.0f }); // Tangent space z positive to rotate around z
		random = DirectX::XMVector3Normalize(random);

		auto lerp = [](float a, float b, float f) {
			return a + f * (b - a);
		};

		/* Advanced */
		float scale = float(i) / ssao_kernel;
		scale = lerp(0.1f, 1.0f, scale * scale);
		random = DirectX::XMVectorScale(random, scale);
		

		DirectX::XMStoreFloat4(&m_constant_buffer_ssao.kernel_samples[i], random);
	}

	// El slider usa las N primeras muestras: sin barajar, con pocas muestras se perderian las de radio grande
	std::shuffle(m_constant_buffer_ssao.kernel_samples, m_constant_buffer_ssao.kernel_samples + ssao_kernel, mt);

	// Noise
	for (unsigned int i = 0; i < ssao_noise; i++) {

		Vec3 noise({ (dist(mt) * 2.0f) -1.0f, (dist(mt) * 2.0f) - 1.0f, 0.0f });
		m_ssao_noise_random.push_back(noise.x);
		m_ssao_noise_random.push_back(noise.y);
		m_ssao_noise_random.push_back(noise.z);
		m_ssao_noise_random.push_back(0.0f);
	}

	// Noise texture
	D3D11_TEXTURE2D_DESC desc = {
		.Width = ssao_noise_dim,
		.Height = ssao_noise_dim,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		.SampleDesc = {
			.Count = 1,
		},
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};

	D3D11_SUBRESOURCE_DATA init_data = {
		.pSysMem = m_ssao_noise_random.data(),
		.SysMemPitch = sizeof(float) * 4 * ssao_noise_dim,
		.SysMemSlicePitch = 0,
	};

	HRESULT hr = m_engine_ptr->get_instance()->get_engine_props()->deviceInterface->CreateTexture2D(&desc, &init_data, &m_noise_texture);
	CheckShaderError(hr);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {
		.Format = desc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MostDetailedMip = 0,
			.MipLevels = 1,
		},
	};
	hr = m_engine_ptr->get_instance()->get_engine_props()->deviceInterface->CreateShaderResourceView(m_noise_texture, &srv_desc, &m_noise_srv);
	CheckShaderError(hr);


	m_constant_buffer_ssao.width = static_cast<float>(m_window->get_window_properties()->width);
	m_constant_buffer_ssao.height = static_cast<float>(m_window->get_window_properties()->height);

	m_engine_ptr->get_instance()->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_ssao, 0, nullptr, &m_constant_buffer_ssao, 0, 0);
}

void Renderer::add_draw_call(){
#ifdef ENABLE_IMGUI
	ImguiManager::get_instance()->m_draw_calls++;
#endif
}

void Renderer::clear_depth(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::clear_render_target(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.postprocess_render_target_view, m_clear_emissive_color);
	
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_material_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_normals_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_albedo_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_layers_render_target_view, m_clear_emissive_color);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.ssao_render_target_view, m_clear_emissive_color);

	for (int i = 0; i < NUM_MIPMAPS_EMISSIVE; i++) {
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], m_clear_emissive_color);
	}
	
}

void Renderer::clear_rtv(int size){

	assert(size <= 5 && "RTV size must be 5 or less");
	ID3D11RenderTargetView* nullRTV[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	auto props = Engine::get_instance()->get_engine_props();

	props->inmediateDeviceContext->OMSetRenderTargets(size, nullRTV, nullptr);
}

void Renderer::clear_srv(int size){

	assert(size <= 5 && "SRV size must be 5 or less");
	ID3D11ShaderResourceView* nullSRV[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	auto props = Engine::get_instance()->get_engine_props();
	props->inmediateDeviceContext->PSSetShaderResources(0, size, nullSRV);
}

void Renderer::set_viewport(unsigned int width, unsigned int height){

	D3D11_VIEWPORT viewport{};
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetViewports(1, &viewport);
}

// ---------------------------------------------------------------------------
// Sondas de reflexion
// ---------------------------------------------------------------------------

void Renderer::init_reflection_probes(){

	auto device = m_engine_ptr->get_engine_props()->deviceInterface;
	auto context = m_engine_ptr->get_engine_props()->inmediateDeviceContext;
	HRESULT hr = S_OK;

	// Captura: cubo HDR con todos los mips, que rellena GenerateMips para el prefiltrado
	D3D11_TEXTURE2D_DESC capture_desc = {};
	capture_desc.Width = PROBE_RESOLUTION;
	capture_desc.Height = PROBE_RESOLUTION;
	capture_desc.MipLevels = 0;
	capture_desc.ArraySize = 6;
	capture_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	capture_desc.SampleDesc.Count = 1;
	capture_desc.Usage = D3D11_USAGE_DEFAULT;
	capture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	capture_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	hr = device->CreateTexture2D(&capture_desc, nullptr, &m_probe_capture_texture);
	assert(SUCCEEDED(hr) && "Failed to create probe capture texture");

	for (unsigned int face = 0; face < 6; face++) {
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.Format = capture_desc.Format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtv_desc.Texture2DArray.MipSlice = 0;
		rtv_desc.Texture2DArray.FirstArraySlice = face;
		rtv_desc.Texture2DArray.ArraySize = 1;
		hr = device->CreateRenderTargetView(m_probe_capture_texture, &rtv_desc, &m_probe_capture_rtv[face]);
		assert(SUCCEEDED(hr) && "Failed to create probe capture RTV");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC capture_srv_desc = {};
	capture_srv_desc.Format = capture_desc.Format;
	capture_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	capture_srv_desc.TextureCube.MostDetailedMip = 0;
	capture_srv_desc.TextureCube.MipLevels = static_cast<UINT>(-1);
	hr = device->CreateShaderResourceView(m_probe_capture_texture, &capture_srv_desc, &m_probe_capture_srv);
	assert(SUCCEEDED(hr) && "Failed to create probe capture SRV");

	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.Width = PROBE_RESOLUTION;
	depth_desc.Height = PROBE_RESOLUTION;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = device->CreateTexture2D(&depth_desc, nullptr, &m_probe_depth_texture);
	assert(SUCCEEDED(hr) && "Failed to create probe depth texture");
	hr = device->CreateDepthStencilView(m_probe_depth_texture, nullptr, &m_probe_dsv);
	assert(SUCCEEDED(hr) && "Failed to create probe DSV");

	// Prefiltradas: un cubo por sonda, en un solo array para leerlas todas en el pase de luces
	D3D11_TEXTURE2D_DESC array_desc = capture_desc;
	array_desc.MipLevels = PROBE_MIPS;
	array_desc.ArraySize = 6 * MAX_REFLECTION_PROBES;
	array_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	hr = device->CreateTexture2D(&array_desc, nullptr, &m_probe_array_texture);
	assert(SUCCEEDED(hr) && "Failed to create probe array texture");

	const float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	for (unsigned int slot = 0; slot < MAX_REFLECTION_PROBES; slot++) {
		for (unsigned int face = 0; face < 6; face++) {
			for (unsigned int mip = 0; mip < PROBE_MIPS; mip++) {
				D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
				rtv_desc.Format = array_desc.Format;
				rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DArray.MipSlice = mip;
				rtv_desc.Texture2DArray.FirstArraySlice = slot * 6 + face;
				rtv_desc.Texture2DArray.ArraySize = 1;
				ID3D11RenderTargetView*& rtv = m_probe_array_rtv[(slot * 6 + face) * PROBE_MIPS + mip];
				hr = device->CreateRenderTargetView(m_probe_array_texture, &rtv_desc, &rtv);
				assert(SUCCEEDED(hr) && "Failed to create probe array RTV");
				context->ClearRenderTargetView(rtv, black);
			}
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC array_srv_desc = {};
	array_srv_desc.Format = array_desc.Format;
	array_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	array_srv_desc.TextureCubeArray.MostDetailedMip = 0;
	array_srv_desc.TextureCubeArray.MipLevels = PROBE_MIPS;
	array_srv_desc.TextureCubeArray.First2DArrayFace = 0;
	array_srv_desc.TextureCubeArray.NumCubes = MAX_REFLECTION_PROBES;
	hr = device->CreateShaderResourceView(m_probe_array_texture, &array_srv_desc, &m_probe_array_srv);
	assert(SUCCEEDED(hr) && "Failed to create probe array SRV");

	D3D11_BUFFER_DESC buffer_desc = {};
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.ByteWidth = sizeof(ReflectionProbeConstantBuffer);
	device->CreateBuffer(&buffer_desc, nullptr, &m_pVBuffer_probes);
	buffer_desc.ByteWidth = sizeof(ProbePrefilterConstantBuffer);
	device->CreateBuffer(&buffer_desc, nullptr, &m_pVBuffer_probe_prefilter);

	ReflectionProbeConstantBuffer empty = {};
	context->UpdateSubresource(m_pVBuffer_probes, 0, nullptr, &empty, 0, 0);
}

void Renderer::release_reflection_probes(){

	const auto free = [](auto*& resource) {
		if (resource) resource->Release();
		resource = nullptr;
	};

	free(m_probe_capture_texture);
	for (auto*& rtv : m_probe_capture_rtv) free(rtv);
	free(m_probe_capture_srv);
	free(m_probe_depth_texture);
	free(m_probe_dsv);
	free(m_probe_array_texture);
	for (auto*& rtv : m_probe_array_rtv) free(rtv);
	free(m_probe_array_srv);
	free(m_pVBuffer_probes);
	free(m_pVBuffer_probe_prefilter);
}

void Renderer::update_reflection_probes(EntityComponentSystem& ecs){

	using namespace DirectX;

	ReflectionProbeConstantBuffer constants = {};
	constants.probe_max_mip = static_cast<float>(PROBE_MIPS - 1);
	const auto upload = [&]() {
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_probes, 0, nullptr, &constants, 0, 0);
	};
	if (!m_probe_array_srv) return;

	// Escena nueva: lo capturado es de la anterior
	Scene* scene = m_engine_ptr->get_scene();
	const std::string scene_path = scene ? scene->get_path() : "";
	if (scene != m_probe_scene || scene_path != m_probe_scene_path) {
		m_probe_scene = scene;
		m_probe_scene_path = scene_path;
		m_probe_scene_time = 0.0f;
		for (ProbeSlot& slot : m_probe_slots) slot.captured = false;
	}

	// Los modelos y las texturas se cargan en otro hilo: lo que ve la sonda cambia al terminar.
	// Se recaptura al acabar de cargar modelos, cuando cambia la geometria y un par de veces al principio.
	bool recapture_all = m_render_info_parameters.probes_recapture;
	m_render_info_parameters.probes_recapture = false;

	const float previous_time = m_probe_scene_time;
	m_probe_scene_time += m_engine_ptr->get_delta_time();
	for (float moment : { 1.0f, 3.0f }) {
		if (previous_time < moment && m_probe_scene_time >= moment) recapture_all = true;
	}

	const bool loading = m_engine_ptr->m_resource.is_loading_models();
	if (m_probe_was_loading && !loading) recapture_all = true;
	m_probe_was_loading = loading;

	if (m_probe_geometry_key != m_depth_geometry_key) {
		m_probe_geometry_key = m_depth_geometry_key;
		recapture_all = true;
	}

	if (!m_render_info_parameters.reflection_probes) {
		upload();
		return;
	}

	struct FoundProbe {
		unsigned int slot;
		Vec3 position;
		ReflectionProbeComponent* probe;
	};
	std::vector<FoundProbe> found;
	for (auto [entity, probe, transform] : ecs.viewComponents<ReflectionProbeComponent, TransformComponent>().each()) {
		if (found.size() >= MAX_REFLECTION_PROBES) break;
		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, transform.get_transform());
		found.push_back({ static_cast<unsigned int>(found.size()), Vec3(world._41, world._42, world._43), &probe });
	}

	std::vector<unsigned int> to_capture;
	for (const FoundProbe& f : found) {
		const ProbeSlot& slot = m_probe_slots[f.slot];
		const bool moved = fabsf(slot.position.x - f.position.x) > 1e-4f || fabsf(slot.position.y - f.position.y) > 1e-4f || fabsf(slot.position.z - f.position.z) > 1e-4f;
		if (recapture_all || !slot.captured || moved) to_capture.push_back(f.slot);
	}

	// En tiempo real, una sonda por frame para repartir el coste
	if (to_capture.empty() && m_render_info_parameters.probes_realtime && !found.empty()) {
		to_capture.push_back(m_probe_realtime_next++ % found.size());
	}

	if (!to_capture.empty()) {
		update_light_buffer(ecs);
		for (unsigned int slot : to_capture) capture_probe(slot, found[slot].position);
	}

	// De la caja mas pequena a la mas grande: una sonda dentro de otra manda en su zona
	std::sort(found.begin(), found.end(), [](const FoundProbe& a, const FoundProbe& b) {
		return a.probe->size.x * a.probe->size.y * a.probe->size.z < b.probe->size.x * b.probe->size.y * b.probe->size.z;
	});

	for (const FoundProbe& f : found) {
		const Vec3 half = { f.probe->size.x * 0.5f, f.probe->size.y * 0.5f, f.probe->size.z * 0.5f };
		GPUReflectionProbe& gpu = constants.probes[constants.probe_count++];
		gpu.box_min = { f.position.x - half.x, f.position.y - half.y, f.position.z - half.z };
		gpu.box_max = { f.position.x + half.x, f.position.y + half.y, f.position.z + half.z };
		gpu.blend = f.probe->blend;
		gpu.intensity = f.probe->intensity;
		gpu.position = f.position;
		gpu.slot = static_cast<float>(f.slot);
	}
	upload();
}

void Renderer::capture_probe(unsigned int slot, const Vec3& position){

	using namespace DirectX;

	auto context = m_engine_ptr->get_engine_props()->inmediateDeviceContext;

	// La captura se va a escribir: que no siga enlazada como textura de un pase anterior
	ID3D11ShaderResourceView* null_srvs[12] = {};
	context->PSSetShaderResources(0, 12, null_srvs);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(PROBE_RESOLUTION);
	viewport.Height = static_cast<float>(PROBE_RESOLUTION);
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// El cielo para los metales y el ambiente, con los mismos mips que el pase de luces
	CameraDeferredConstantBuffer deferred = {};
	deferred.inv_view_proj = XMMatrixIdentity();
	deferred.camera_position = position;
	deferred.cubemap_max_mip_level = static_cast<float>(m_engine_ptr->m_resource.get_specular_max_miplevel());
	context->UpdateSubresource(m_pVBufferDeferredConstantCamera, 0, nullptr, &deferred, 0, 0);

	// Una camara de 90 grados por cara, con el convenio de caras de D3D: +X, -X, +Y, -Y, +Z, -Z
	const Mat4 projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.05f, 500.0f);
	const XMFLOAT3 looks[6] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
	const XMFLOAT3 ups[6] = { { 0, 1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { 0, 1, 0 }, { 0, 1, 0 } };
	const FVector eye = XMLoadFloat3(&position);
	const float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	for (unsigned int face = 0; face < 6; face++) {
		const Mat4 view = XMMatrixLookToLH(eye, XMLoadFloat3(&looks[face]), XMLoadFloat3(&ups[face]));

		context->ClearRenderTargetView(m_probe_capture_rtv[face], black);
		context->ClearDepthStencilView(m_probe_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
		context->OMSetRenderTargets(1, &m_probe_capture_rtv[face], m_probe_dsv);
		context->OMSetDepthStencilState(m_depth_only_state, 1);
		context->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
		context->RSSetState(m_engine_ptr->m_raster_state);
		context->IASetInputLayout(m_pLayout);
		context->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);
		context->PSSetConstantBuffers(0, 1, &m_pVBuffer_light_pass);
		context->PSSetConstantBuffers(1, 1, &m_pVBufferDeferredConstantCamera);

		ID3D11ShaderResourceView* environment[] = {
			m_engine_ptr->m_resource.get_specular_srv(),
			m_light_buffer_srv,
			m_engine_ptr->m_resource.get_irradiance_srv(),
		};
		context->PSSetShaderResources(6, ARRAYSIZE(environment), environment);
		active_shader(ShaderType::ProbeCapture);

		CameraConstantBuffer camera = {};
		camera.view = XMMatrixTranspose(view);
		camera.projection = XMMatrixTranspose(projection);
		camera.camera_position = position;
		for (VisibleMesh& mesh : m_scene_meshes) {
			render_mesh_internal(&camera, *mesh.transform, *mesh.mesh);
		}

		draw_skybox_to(m_probe_capture_rtv[face], m_probe_dsv, view, projection, position);
	}

	context->OMSetRenderTargets(0, nullptr, nullptr);
	context->PSSetShaderResources(0, 12, null_srvs);
	context->GenerateMips(m_probe_capture_srv);

	// Prefiltrado: cada mip del cubo de la sonda es el reflejo para una rugosidad
	active_shader(ShaderType::ProbePrefilter);
	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;
	context->IASetInputLayout(m_pLayout_deferred);
	context->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->OMSetDepthStencilState(m_depth_stencil_ssao, 0);
	context->RSSetState(m_engine_ptr->m_raster_state);
	context->PSSetSamplers(0, 1, &m_sampler_state_emissive);
	context->PSSetShaderResources(0, 1, &m_probe_capture_srv);
	context->PSSetConstantBuffers(0, 1, &m_pVBuffer_probe_prefilter);

	for (unsigned int mip = 0; mip < PROBE_MIPS; mip++) {
		const unsigned int size = std::max(1u, PROBE_RESOLUTION >> mip);
		viewport.Width = static_cast<float>(size);
		viewport.Height = static_cast<float>(size);
		context->RSSetViewports(1, &viewport);

		for (unsigned int face = 0; face < 6; face++) {
			ProbePrefilterConstantBuffer prefilter = {};
			prefilter.face = face;
			prefilter.roughness = static_cast<float>(mip) / static_cast<float>(PROBE_MIPS - 1);
			prefilter.face_size = static_cast<float>(size);
			prefilter.source_resolution = static_cast<float>(PROBE_RESOLUTION);
			context->UpdateSubresource(m_pVBuffer_probe_prefilter, 0, nullptr, &prefilter, 0, 0);

			context->OMSetRenderTargets(1, &m_probe_array_rtv[(slot * 6 + face) * PROBE_MIPS + mip], nullptr);
			context->Draw(3, 0);
			add_draw_call();
		}
	}

	context->OMSetRenderTargets(0, nullptr, nullptr);
	context->PSSetShaderResources(0, 1, null_srvs);
	context->RSSetViewports(1, &m_engine_ptr->m_viewport);

	m_probe_slots[slot].captured = true;
	m_probe_slots[slot].position = position;
}
