#include <render/renderer.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <algorithm>

#include "render/imgui/imgui_manager.h"


static inline bool  CheckShaderError(HRESULT hr, ID3DBlob* error_msg = nullptr) {
	if (FAILED(hr)) {
		if (error_msg != nullptr) {
			OutputDebugStringA((char*)error_msg->GetBufferPointer());
			error_msg->Release();
		}
			return false;
	}

	return true;
}

Renderer::Renderer() : m_shader_files(), m_sampler_desc{} {
	m_isInitialized = false;
	m_pVBuffer = nullptr;
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

Renderer::Renderer(Renderer&&){

}

Renderer::~Renderer(){
	m_depth_buffer->Release();
	m_depth_stencil_view->Release();
	m_pVBufferConstantCamera->Release();
	m_pVBufferDeferredConstantCamera->Release();
	m_pVBuffer->Release();
	m_pVBuffer_full_triangle->Release();

	m_pLayout->Release();
	m_sampler_state->Release();
	m_depth_stencil_state->Release();

}

bool Renderer::init_pipeline(Window* win){

	m_engine_ptr = Engine::get_instance();
	m_window = win;

	ID3DBlob* VS = nullptr;
	ID3DBlob* VS_deferred = nullptr;
	ID3DBlob* VS_skybox = nullptr;
	ID3DBlob* VS_depth = nullptr;
	ID3DBlob* PS = nullptr;
	ID3DBlob* error_msg = nullptr;
	
	if (!std::filesystem::exists("data/shaders/forward/vs_common.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/vs_common.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/forward/ps_directional.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/ps_directional.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/forward/ps_point.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/ps_point.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/forward/ps_spot.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/ps_spot.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/deferred/vs_deferred.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/deferred/vs_deferred.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/deferred/ps_deferred.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/deferred/ps_deferred.hlsl");
	}

	// Forward rendering shaders

	HRESULT hr = D3DCompileFromFile(L"data/shaders/forward/vs_common.hlsl", nullptr, nullptr, "VShader", m_vertex_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_shader_files.VS_common);


	hr = D3DCompileFromFile(L"data/shaders/forward/ps_directional.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_directional);

	hr = D3DCompileFromFile(L"data/shaders/forward/ps_point.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_point);

	hr = D3DCompileFromFile(L"data/shaders/forward/ps_spot.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_spot);
	


	// Deferred rendering shaders

	
	// Vertex shader geometry pass
	hr = D3DCompileFromFile(L"data/shaders/deferred/vs_deferred.hlsl", nullptr, nullptr, "VShader", m_vertex_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),NULL, &m_shader_files.VS_deferred);
	
	// Pixel shader geometry pass
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred);

	// Light pass vertex shader
	hr = D3DCompileFromFile(L"data/shaders/deferred/vs_deferred_common.hlsl", nullptr, nullptr, "VShader", m_vertex_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS_deferred, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_deferred->GetBufferPointer(), VS_deferred->GetBufferSize(), NULL, &m_shader_files.VS_deferred_common);

	// Light pass pixel shader directional
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_directional.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_directional);

	// Light pass pixel shader point light
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_point.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_point);

	// Light pass pixel shader spot light
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_spot.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_spot);

	// Light pass pixel shader Passthrough
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_passthrough.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_passthrough);
	
	// Light pass pixel shader PostProcess
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_post_process_bloom.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_emissive);

	// Bloom downscaling
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_post_process_bloom_downscaling.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_emissive_downsample);
	
	
	
	// Skybox VS
	hr = D3DCompileFromFile(L"data/shaders/deferred/vs_deferred_skybox.hlsl", nullptr, nullptr, "VShader", m_vertex_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS_skybox, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_skybox->GetBufferPointer(), VS_skybox->GetBufferSize(), NULL, &m_shader_files.VS_deferred_skybox);

	// Skybox PS
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_skybox.hlsl", nullptr, nullptr, "PShader", m_pixel_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_skybox);

	// Depth prepass VS
	hr = D3DCompileFromFile(L"data/shaders/vs_depth_prepass.hlsl", nullptr, nullptr, "VShader", m_vertex_shader_model.c_str(), D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS_depth, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_depth->GetBufferPointer(), VS_depth->GetBufferSize(), NULL, &m_shader_files.VS_depth_prepass);

	
	
	

	/**** Buffers creation ****/
	ZeroMemory(&m_buffer_description, sizeof(m_buffer_description));
	m_buffer_description.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
	m_buffer_description.ByteWidth = sizeof(Vertex) * 3;
	m_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// Using as Vertex buffer
	m_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_buffer_description, NULL, &m_pVBuffer);
	
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


	/**** Depth stencil and texture creation ****/
	D3D11_TEXTURE2D_DESC depth_desc{
		.Width = win->get_window_properties()->width,
		.Height = win->get_window_properties()->height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc{
			.Count = 1
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL
	};
	m_depth_buffer = nullptr;
	m_depth_stencil_view = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateTexture2D(&depth_desc, nullptr, &m_depth_buffer);
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilView(m_depth_buffer, nullptr, &m_depth_stencil_view);
	//m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilView(m_depth_buffer, nullptr, nullptr);



	create_backbuffers();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &(win->get_window_info()->backbuffer), m_depth_stencil_view);	// last argument is depth stencill view

	
	// Setting the emissive buffer
	//win->get_window_info()->emissive_buffer_view = nullptr;
	//hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateRenderTargetView(m_emissive_texture, NULL, (&(win->get_window_info()->emissive_buffer_view)));
	//CheckShaderError(hr);

	//hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateShaderResourceView(m_emissive_texture, nullptr, &m_emissive_SRV);
	//CheckShaderError(hr);



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

	
	// Depth stencil for skybox
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc_skybox = {
	  .DepthEnable = true,
	  .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
	  .DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
	  .StencilEnable = false,
	  .StencilReadMask = 0,
	  .StencilWriteMask = 0,
	  .FrontFace = {},
	  .BackFace = {}
	};
	m_depth_stencil_state_skybox = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_stencil_desc_skybox, &m_depth_stencil_state_skybox);

	D3D11_DEPTH_STENCIL_DESC depth_desc_prepass = {
		.DepthEnable = true,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
		.StencilEnable = false,
	};
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_desc_prepass, &m_depth_only_state);
	
	

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
	CheckShaderError(hr, error_msg);

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
	CheckShaderError(hr, error_msg);


	// Blend for skybox
	D3D11_BLEND_DESC blend_skybox_desc = {
		 .AlphaToCoverageEnable = false,
		 .IndependentBlendEnable = false,
		 .RenderTarget = {
		   {
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
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBlendState(&blend_skybox_desc, &m_blend_skybox);



	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBlendState(&blendDesc, &m_blend_off);
	
	// Skybox buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(m_skybox_vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData_skybox = {};
	initData_skybox.pSysMem = m_skybox_vertices;

	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&bd, &initData_skybox, &m_pVBuffer_skybox_buffer);


	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, m_blend_factor, m_blend_mask);


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
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied_skybox, 1, VS_depth->GetBufferPointer(), VS_depth->GetBufferSize(), &m_pLayout_depth);
	CheckShaderError(hr);
	//m_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	m_sampler_desc.MaxAnisotropy = 2;

	m_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	m_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//m_sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;	// For shadow mapping
	
	m_sampler_desc.MinLOD = 0;
	m_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_sampler_state = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateSamplerState(&m_sampler_desc, &m_sampler_state);

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



	return m_isInitialized;
}

void Renderer::active_shader(ShaderType type){
	switch (type){
	case ShaderType::GeometryPass:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred, nullptr, 0);
		break;
	
	case ShaderType::DeferredDirectional:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_directional, nullptr, 0);
		break;
	
	case ShaderType::DeferredPoint:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_point, nullptr, 0);
		break;
	
	case ShaderType::DeferredSpot:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_spot, nullptr, 0);
		break;

	case ShaderType::DeferredPassThrough:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_passthrough, nullptr, 0);
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
	case ShaderType::DeferredEmissive:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_emissive, nullptr, 0);
		break;
	case ShaderType::BloomDownsample:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_emissive_downsample, nullptr, 0);
		break;
	case ShaderType::Skybox:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_deferred_skybox, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_deferred_skybox, nullptr, 0);
		break;
	case ShaderType::Depth:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_depth_prepass, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(nullptr, nullptr, 0);
		break;

	default:break;
	}
}

void Renderer::enable_depth_prepass(bool enabled){

	m_depth_prepass = enabled;
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


	
	//++m_buffer_index %= NUM_BUFFERING;


#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float, std::milli>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
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

	clear_render_target();
	clear_depth();

#ifdef DEPTH_PREPASS
	depth_pass(ecs);
#endif // DEPTH_PREPASS

	active_shader(ShaderType::GeometryPass);

	ID3D11RenderTargetView* gbuffer_rtv[] = {
		m_deferred_resources.gbuffer_albedo_render_target_view,
		m_deferred_resources.gbuffer_position_render_target_view,
		m_deferred_resources.gbuffer_normals_render_target_view,
		m_deferred_resources.gbuffer_material_render_target_view,
		m_deferred_resources.gbuffer_emissive_render_target_view,
	};

	auto props = Engine::get_instance()->get_engine_props();

	// Unbind render targets
	clear_shader_reources();

	// Clear render targets
	for (int i = 0; i < ARRAYSIZE(gbuffer_rtv); ++i) props->inmediateDeviceContext->ClearRenderTargetView(gbuffer_rtv[i], m_clear_emissive_color);

	

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state, 1);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->RSSetState(m_current_raster);

	// Geometry pass
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);

	

	
	// Bind render targets
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(gbuffer_rtv), gbuffer_rtv, m_depth_stencil_view);



	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	auto directional_light = ecs.viewComponents<DirectionalLight>();
	auto point_light = ecs.viewComponents<PointLight>();
	auto spot_light = ecs.viewComponents<SpotLight>();

	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();

	CameraConstantBuffer cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);
	cam_buffer.camera_position = m_cam->get_position();
	

	for (auto [entity, trans, mesh] : transforms.each()) {

		for (Mesh& m : mesh.get_model()->meshes) {
			render_mesh_internal(&cam_buffer, trans, m);
		}
	}
	// Unbind render targets
	clear_shader_reources();


	// Light Pass
	float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	props->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.postprocess_render_target_view, clear_color);


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(1, 1, &m_pVBufferDeferredConstantCamera);


	ID3D11RenderTargetView* light_pass_rtvs[] = {
		m_deferred_resources.postprocess_render_target_view, // Final color
		m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[0]	// first mipmap level
	};
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(light_pass_rtvs), light_pass_rtvs, m_depth_stencil_view); // no m_depth_stencil_view, is a full screen triangle 
	//props->inmediateDeviceContext->OMSetRenderTargets(1, &m_quad_RTV, m_depth_stencil_view);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;

	ID3D11ShaderResourceView* srvs[] = {
		m_deferred_resources.gbuffer_albedo_shader_resource_view,
		m_deferred_resources.gbuffer_position_shader_resource_view,
		m_deferred_resources.gbuffer_normals_shader_resource_view,
		m_deferred_resources.gbuffer_material_shader_resource_view,
		m_deferred_resources.gbuffer_emissive_shader_resource_view,
		m_engine_ptr->m_resource.get_skybox_srv(),
		m_engine_ptr->m_resource.get_brdf_srv(),

	};
	props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	CameraDeferredConstantBuffer cam_buffer_light;
	Mat4 view_proj = (*view) * (*proj);
	cam_buffer_light.inv_view_proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, view_proj));;
	cam_buffer_light.camera_position = m_cam->get_position();
	cam_buffer_light.cubemap_max_mip_level = m_engine_ptr->m_resource.get_skybox_max_miplevel();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBufferDeferredConstantCamera, 0, nullptr, &cam_buffer_light, 0, 0);


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);


	
	active_shader(ShaderType::DeferredDirectional);
	for (auto [entity, light] : directional_light.each()) {
		if (light.get_enabled()) {

			light.update();
			light.upload_data();
	
			props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
			props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			props->inmediateDeviceContext->Draw(3, 0);
			add_draw_call();
			m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
		}
	}

	
	active_shader(ShaderType::DeferredPoint);
	
	for (auto [entity, light] : point_light.each()) {
		if (light.get_enabled()) {

			light.update();
			light.upload_data();
	
			props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
			props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			props->inmediateDeviceContext->Draw(3, 0);
			add_draw_call();
			m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
		}
	}
	/*
	
	active_shader(ShaderType::DeferredSpot);
	for (auto [entity, light] : spot_light.each()) {
		light.update();
		light.upload_data();
	
		props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
		props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		props->inmediateDeviceContext->Draw(3, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
	}
	*/

	



	// Unbind light pass rtv
	//ID3D11RenderTargetView* nullRTV[] = { nullptr, nullptr };

	
	clear_shader_reources();
	props->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_out_b_render_target_view, m_clear_emissive_color);

	draw_skybox();

	//draw_emissive();
	draw_emissive_downsample();

	clear_srv(2);

	// Draw in the back buffer
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);

	// Setear textura resultado (SRV)
	if (m_bloom_active) {
		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[0]);
	}else {
		props->inmediateDeviceContext->PSSetShaderResources(0, 1, &(m_deferred_resources.postprocess_resource_view));
	}

	//props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_quad_SRV);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	// Dibujar quad
	active_shader(ShaderType::DeferredPassThrough);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);
	add_draw_call();

	// Limpiar SRV
	ID3D11ShaderResourceView* nullSRV = nullptr;
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &nullSRV);




#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float, std::milli>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	m_bloom_active = ImguiManager::get_instance()->m_bloom;
	set_draw_mode(ImguiManager::get_instance()->m_current_draw_mode);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float, std::milli>(elapsed_imgui).count();

#endif


}

void Renderer::set_draw_mode(DrawMode mode){

	switch (mode) {
		case DrawMode::Solid: m_current_raster = m_engine_ptr->m_raster_state; break;
		case DrawMode::Wireframe: m_current_raster = m_engine_ptr->m_raster_state_wireframe; break;
	}

	m_current_draw_mode = mode;
	ImguiManager::get_instance()->m_current_draw_mode = mode;
}

void Renderer::set_cull_mode(){

}

void Renderer::set_camera(CameraComponent* cam){
	assert(cam != nullptr);

	m_cam = cam;
}

DrawMode Renderer::get_current_draw_mode(){

	return m_current_draw_mode;
}

void Renderer::release(){
	m_shader_files.VS_common->Release();
	m_shader_files.PS_directional->Release();
	m_shader_files.PS_point->Release();
	m_shader_files.PS_spot->Release();
}

void Renderer::render_mesh_internal(CameraConstantBuffer* camera_buffer, TransformComponent& trans, Mesh& m){
	
	// Set camera values to constant buffer
	camera_buffer->model = DirectX::XMMatrixTranspose(trans.get_transform());

	// Upload metallic and roughness values to constant buffer
	camera_buffer->metallic = m.material.get_metallic_value();
	camera_buffer->roughness = m.material.get_roughness_value();
	camera_buffer->color = m.material.get_color_value();
	camera_buffer->emissive = m.material.get_emissive_value();
	camera_buffer->emissive_intensity = m.material.get_emissive_intensity();
	
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
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->DrawIndexed(m.num_indices, 0, 0);
	add_draw_call();
}

void Renderer::render_deferred_internal(){

}

void Renderer::draw_emissive(){

	if (m_bloom_active) {

		UINT stride = sizeof(VertexQuad);
		UINT offset = 0;
		
		auto props = Engine::get_instance()->get_engine_props();
		
		clear_shader_reources();

		// Draw postprocess emissive Horizontal
		ID3D11RenderTargetView* postprocess_pass_rtvs[] = {
			m_deferred_resources.gbuffer_emissive_out_b_render_target_view,
		};

		ID3D11ShaderResourceView* srvs_post_process[] = {
			m_deferred_resources.gbuffer_emissive_out_shader_resource_view,
			//m_deferred_resources.postprocess_resource_view, // first time is not used
		};

		active_shader(ShaderType::DeferredEmissive);
		props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(postprocess_pass_rtvs), postprocess_pass_rtvs, m_depth_stencil_view);
		props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs_post_process), srvs_post_process);
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		// Set emissive constant buffer
		EmissiveConstantBuffer emissive_buffer;
		emissive_buffer.texel_size = { 1.0f / m_window->m_width, 1.0f / m_window->m_height };
		emissive_buffer.bloom_intensity = 10.0f;
		emissive_buffer.horizontal = 1;
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);


		// gbuffer_emissive_out_b_render_target_view is empty, need to overwritte
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
		props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
		props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
		props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();

		clear_shader_reources();

		// Draw postprocess emissive Vertical
		ID3D11RenderTargetView* postprocess_pass_vertical_rtvs[] = {
			m_quad_RTV,
		};

		ID3D11ShaderResourceView* srvs_post_process_vertical[] = {
			m_deferred_resources.gbuffer_emissive_out_b_shader_resource_view,
			m_deferred_resources.postprocess_resource_view, // Light color previously calculated
		};


		props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs_post_process_vertical), srvs_post_process_vertical);

		//active_shader(ShaderType::DeferredEmissive);
		props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(postprocess_pass_vertical_rtvs), postprocess_pass_vertical_rtvs, m_depth_stencil_view);
		props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs_post_process_vertical), srvs_post_process_vertical);
		props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state_emissive);

		// Set emissive constant buffer
		emissive_buffer.horizontal = 0;

		m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBuffer_emissive_constant_buffer, 0, nullptr, &emissive_buffer, 0, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBuffer_emissive_constant_buffer);

		//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
		props->inmediateDeviceContext->Draw(3, 0);
		add_draw_call();

	}
}

void Renderer::draw_emissive_downsample(){

	// TODO: Crear Texturas temporales para cada mipmap level, no puedo escribir el blur horizontal siempre en gbuffer_emissive_out porque es del tamaño del original, tiene que ser del tamaño del nuevo mip

	auto props = m_engine_ptr->get_engine_props();

	//clear_render_target();
	//clear_shader_reources();
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.emissive_dowscaling_render_target_view[i - 1], m_clear_emissive_color);
	

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
		// Horizontal blur
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
		// Horizontal blur
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
		}else {
			props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_deferred_resources.emissive_dowscaling_shader_resource_view[i + 1]);
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

void Renderer::depth_pass(EntityComponentSystem& ecs){
	auto props = m_engine_ptr->get_engine_props();
	props->inmediateDeviceContext->OMSetRenderTargets(0, nullptr, m_depth_stencil_view);
	props->inmediateDeviceContext->OMSetDepthStencilState(m_depth_only_state, 1);
	props->inmediateDeviceContext->RSSetState(m_current_raster);

	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_depth);
	props->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBuffer_constant_camera_depth_only);
	active_shader(ShaderType::Depth);
	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();

	CameraDepthPrePass cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);

	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	for (auto [entity, trans, mesh] : transforms.each()) {
		for (Mesh& m : mesh.get_model()->meshes) {
			render_mesh_depth_only(&cam_buffer, trans, m); // versión simplificada sin materiales
		}
	}

}

void Renderer::draw_skybox(){
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
	props->inmediateDeviceContext->OMSetRenderTargets(1, &m_deferred_resources.postprocess_render_target_view, m_depth_stencil_view);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	const Mat4* view = m_cam->get_view();
	const Mat4* proj = m_cam->get_projection();

	CameraConstantBuffer cam_buffer;
	cam_buffer.view = DirectX::XMMatrixTranspose(*view);
	cam_buffer.projection = DirectX::XMMatrixTranspose(*proj);
	cam_buffer.camera_position = m_cam->get_position();
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

void Renderer::create_deferred_resources(unsigned int width, unsigned int height){
	Engine* e = Engine::get_instance();
	auto device = e->get_engine_props()->deviceInterface;

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

	// Albedo (RGBA8 is usually enough)
	create_render_target(DXGI_FORMAT_R8G8B8A8_UNORM,
		&m_deferred_resources.gbuffer_albedo_texture,
		&m_deferred_resources.gbuffer_albedo_render_target_view,
		&m_deferred_resources.gbuffer_albedo_shader_resource_view
	);
	
	// Position
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_position_texture,
		&m_deferred_resources.gbuffer_position_render_target_view,
		&m_deferred_resources.gbuffer_position_shader_resource_view
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
	
	// Emissive (Output)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_emissive_out_texture,
		&m_deferred_resources.gbuffer_emissive_out_render_target_view,
		&m_deferred_resources.gbuffer_emissive_out_shader_resource_view
	);
	
	// Emissive B (Output)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_emissive_out_b_texture,
		&m_deferred_resources.gbuffer_emissive_out_b_render_target_view,
		&m_deferred_resources.gbuffer_emissive_out_b_shader_resource_view
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

	// Light accumulation buffer (HDR format)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.light_texture,
		&m_deferred_resources.light_render_target_view,
		&m_deferred_resources.light_shader_resource_view
	);
	
	// PostProcess
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.postprocess_texture,
		&m_deferred_resources.postprocess_render_target_view,
		&m_deferred_resources.postprocess_resource_view
	);


}

void Renderer::release_deferred_resources(){

	auto func = [&](ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRTV, ID3D11ShaderResourceView** outSRV) {

		if (*outTex) (*outTex)->Release(); (*outTex) = nullptr;
		if (*outRTV) (*outRTV)->Release(); (*outRTV) = nullptr;
		if (*outSRV) (*outSRV)->Release(); (*outSRV) = nullptr;
	};

	func(&m_deferred_resources.gbuffer_albedo_texture, &m_deferred_resources.gbuffer_albedo_render_target_view, &m_deferred_resources.gbuffer_albedo_shader_resource_view);
	func(&m_deferred_resources.gbuffer_position_texture, &m_deferred_resources.gbuffer_position_render_target_view, &m_deferred_resources.gbuffer_position_shader_resource_view);
	func(&m_deferred_resources.gbuffer_normals_texture, &m_deferred_resources.gbuffer_normals_render_target_view, &m_deferred_resources.gbuffer_normals_shader_resource_view);
	func(&m_deferred_resources.gbuffer_material_texture, &m_deferred_resources.gbuffer_material_render_target_view, &m_deferred_resources.gbuffer_material_shader_resource_view);
	func(&m_deferred_resources.gbuffer_emissive_texture, &m_deferred_resources.gbuffer_emissive_render_target_view, &m_deferred_resources.gbuffer_emissive_shader_resource_view);

	func(&m_deferred_resources.gbuffer_emissive_out_texture, &m_deferred_resources.gbuffer_emissive_out_render_target_view, &m_deferred_resources.gbuffer_emissive_out_shader_resource_view);
	func(&m_deferred_resources.gbuffer_emissive_out_b_texture, &m_deferred_resources.gbuffer_emissive_out_b_render_target_view, &m_deferred_resources.gbuffer_emissive_out_b_shader_resource_view);
	
	for (unsigned int i = 0; i < NUM_MIPMAPS_EMISSIVE; i++) {
		func(&m_deferred_resources.gbuffer_emissive_mipmap_texture[i], &m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], &m_deferred_resources.gbuffer_emissive_mipmap_shader_resource_view[i]);
		func(&m_deferred_resources.emissive_dowscaling_texture[i], &m_deferred_resources.emissive_dowscaling_render_target_view[i], &m_deferred_resources.emissive_dowscaling_shader_resource_view[i]);
	}
	
	func(&m_deferred_resources.light_texture, &m_deferred_resources.light_render_target_view,&m_deferred_resources.light_shader_resource_view);
	func(&m_deferred_resources.postprocess_texture, &m_deferred_resources.postprocess_render_target_view,&m_deferred_resources.postprocess_resource_view);
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
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_window->get_window_info()->emissive_buffer_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_out_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_out_b_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.postprocess_render_target_view, m_clear_emissive_color);
	
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_material_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_normals_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_position_render_target_view, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_albedo_render_target_view, m_clear_emissive_color);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_quad_RTV, m_clear_emissive_color);

	for (int i = 0; i < NUM_MIPMAPS_EMISSIVE; i++) {
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_deferred_resources.gbuffer_emissive_mipmap_render_target_view[i], m_clear_emissive_color);
	}
}

void Renderer::clear_full_quad(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_quad_RTV, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::compile_shader(std::string path){

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
