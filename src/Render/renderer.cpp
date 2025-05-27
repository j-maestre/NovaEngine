#include <render/renderer.h>
#include <d3dcompiler.h>
#include <filesystem>

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
	m_clear_emissive_color[0] = 0.3f;
	m_clear_emissive_color[1] = 0.3f;
	m_clear_emissive_color[2] = 0.3f;
	m_clear_emissive_color[3] = 0.3f;

	m_fs_quad[0] = { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} };
	m_fs_quad[1] = { {-1.0f,  3.0f, 0.0f}, {0.0f, -1.0f} };
	m_fs_quad[2] = { { 3.0f, -1.0f, 0.0f}, {2.0f, 1.0f} };
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

	HRESULT hr = D3DCompileFromFile(L"data/shaders/forward/vs_common.hlsl", nullptr, nullptr, "VShader", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_shader_files.VS_common);


	hr = D3DCompileFromFile(L"data/shaders/forward/ps_directional.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_directional);

	hr = D3DCompileFromFile(L"data/shaders/forward/ps_point.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_point);

	hr = D3DCompileFromFile(L"data/shaders/forward/ps_spot.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_spot);
	


	// Deferred rendering shaders

	// Vertex shader geometry pass
	hr = D3DCompileFromFile(L"data/shaders/deferred/vs_deferred.hlsl", nullptr, nullptr, "VShader", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),NULL, &m_shader_files.VS_deferred);
	
	// Pixel shader geometry pass
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred);

	// Light pass vertex shader
	hr = D3DCompileFromFile(L"data/shaders/deferred/vs_deferred_common.hlsl", nullptr, nullptr, "VShader", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS_deferred, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS_deferred->GetBufferPointer(), VS_deferred->GetBufferSize(), NULL, &m_shader_files.VS_deferred_common);

	// Light pass pixel shader directional
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_directional.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_directional);

	// Light pass pixel shader Passthrough
	hr = D3DCompileFromFile(L"data/shaders/deferred/ps_deferred_passthrough.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_shader_files.PS_deferred_passthrough);





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

	// Camera deferred constant buffer creation
	ZeroMemory(&m_cam_deferred_constant_buffer, sizeof(m_cam_deferred_constant_buffer));
	m_cam_deferred_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_cam_deferred_constant_buffer.ByteWidth = sizeof(CameraDeferredConstantBuffer);
	m_cam_deferred_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_cam_deferred_constant_buffer.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cam_deferred_constant_buffer,NULL, &m_pVBufferDeferredConstantCamera);


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


	// Setting the back buffer
	m_engine_ptr->get_engine_props()->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_backbuffer_texture);									// __uuidof gets the unique id of the COM object
	m_engine_ptr->get_engine_props()->deviceInterface->CreateRenderTargetView(m_backbuffer_texture, NULL, &(win->get_window_info()->backbuffer));
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
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,		// For multiple lights
		//.DepthFunc = D3D11_COMPARISON_LESS,			// For one light only	
	};
	m_depth_stencil_state = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateDepthStencilState(&depth_stencil_desc, &m_depth_stencil_state);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetDepthStencilState(m_depth_stencil_state,1);


	// Blend state

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
	case ShaderType::Emissive:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_common, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_emissive, nullptr, 0);
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

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);


	



#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float>(elapsed_imgui).count();

#endif

}

void Renderer::render_deferred(EntityComponentSystem& ecs){


#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif

	ID3D11ShaderResourceView* nullSRVs[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };


	// Geometry pass
	clear_depth();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);

	
	ID3D11RenderTargetView* gbuffer_rtv[] = {
		m_deferred_resources.gbuffer_albedo_render_target_view,
		m_deferred_resources.gbuffer_position_render_target_view,
		m_deferred_resources.gbuffer_normals_render_target_view,
		m_deferred_resources.gbuffer_material_render_target_view,
		m_deferred_resources.gbuffer_emissive_render_target_view,
	};

	auto props = Engine::get_instance()->get_engine_props();
	
	// Unbind render targets
	props->inmediateDeviceContext->PSSetShaderResources(0, 5, nullSRVs);

	// Clear render targets
	for (int i = 0; i < ARRAYSIZE(gbuffer_rtv); ++i) props->inmediateDeviceContext->ClearRenderTargetView(gbuffer_rtv[i], m_clear_emissive_color);
	
	// Bind render targets
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(gbuffer_rtv), gbuffer_rtv, m_depth_stencil_view);


	active_shader(ShaderType::GeometryPass);

	auto transforms = ecs.viewComponents<TransformComponent, MeshComponent>();
	auto directional_light = ecs.viewComponents<DirectionalLight>();

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
	ID3D11RenderTargetView* nullRTVs[ARRAYSIZE(gbuffer_rtv)] = { nullptr };
	props->inmediateDeviceContext->OMSetRenderTargets(ARRAYSIZE(gbuffer_rtv), nullRTVs, nullptr);;


	// Light Pass

	float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	props->inmediateDeviceContext->ClearRenderTargetView(m_quad_RTV, clear_color);

	active_shader(ShaderType::DeferredDirectional);


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetConstantBuffers(0, 1, &m_pVBufferDeferredConstantCamera);
	props->inmediateDeviceContext->OMSetRenderTargets(1, &m_quad_RTV, m_depth_stencil_view);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;

	ID3D11ShaderResourceView* srvs[] = {
		m_deferred_resources.gbuffer_albedo_shader_resource_view,
		m_deferred_resources.gbuffer_position_shader_resource_view,
		m_deferred_resources.gbuffer_normals_shader_resource_view,
		m_deferred_resources.gbuffer_material_shader_resource_view,
		m_deferred_resources.gbuffer_emissive_shader_resource_view
	};
	props->inmediateDeviceContext->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	CameraDeferredConstantBuffer cam_buffer_light;
	Mat4 view_proj = (*view) * (*proj);
	cam_buffer_light.inv_view_proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, view_proj));;
	cam_buffer_light.camera_position = cam_buffer.camera_position;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBufferDeferredConstantCamera, 0, nullptr, &cam_buffer_light, 0, 0);

	for (auto [entity, light] : directional_light.each()) {
		light.update();
		light.upload_data();
	
		props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
		props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		props->inmediateDeviceContext->Draw(3, 0);
	}


	// Draw in the back buffer
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);

	// Setear textura resultado (SRV)
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &m_quad_SRV);
	props->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);

	// Dibujar quad
	active_shader(ShaderType::DeferredPassThrough);
	props->inmediateDeviceContext->IASetInputLayout(m_pLayout_deferred);
	props->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer_full_triangle, &stride, &offset);
	props->inmediateDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	props->inmediateDeviceContext->Draw(3, 0);

	// Limpiar SRV
	ID3D11ShaderResourceView* nullSRV = nullptr;
	props->inmediateDeviceContext->PSSetShaderResources(0, 1, &nullSRV);




#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float>(elapsed_imgui).count();

#endif


}

void Renderer::render_emissive(EntityComponentSystem& ecs){

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif

	clear_depth();
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetConstantBuffers(0, 1, &m_pVBufferConstantCamera);

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

	//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };



	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->emissive_buffer_view, m_depth_stencil_view);
	clear_emissive();


	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_quad_RTV, m_depth_stencil_view);
	clear_full_quad();

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

	clear_emissive();
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

	clear_emissive();
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

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);


	render_full_screen_quad();




#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float>(elapsed).count();


	auto start_imgui = std::chrono::high_resolution_clock::now();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
	auto end_imgui = std::chrono::high_resolution_clock::now();
	auto elapsed_imgui = end_imgui - start_imgui;
	ImguiManager::get_instance()->m_draw_imgui_time = std::chrono::duration<float>(elapsed_imgui).count();

#endif
}

void Renderer::set_cull_mode(){

}

void Renderer::set_camera(CameraComponent* cam){
	assert(cam != nullptr);

	m_cam = cam;
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

	// Draw
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetIndexBuffer(m.index_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->DrawIndexed(m.num_indices, 0, 0);
}

void Renderer::render_deferred_internal(){

}

void Renderer::render_full_screen_quad(){

	// Bind el backbuffer para dibujar el fullscreen quad final
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, nullptr);

	// Bind los shader resource views (escena y emissive) al pixel shader
	ID3D11ShaderResourceView* srvs[] = { m_quad_SRV, m_emissive_SRV };
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(0, 2, srvs);

	// Bind el shader que combina las texturas (asegúrate de tenerlo activo)
	active_shader(ShaderType::Emissive);

	// Bind y dibuja el fullscreen quad (vertex buffer, input layout, draw call)
	//render_fullscreen_quad();

	// Limpia los shader resource views para evitar warnings y conflictos
	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(0, 2, nullSRVs);
}

void Renderer::create_deferred_resources(unsigned int width, unsigned int height){
	Engine* e = Engine::get_instance();
	auto device = e->get_engine_props()->deviceInterface;

	auto create_render_target = [&](DXGI_FORMAT format, ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRTV, ID3D11ShaderResourceView** outSRV) {
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
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
	create_render_target(DXGI_FORMAT_R8G8B8A8_UNORM,
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

	// Material info (Metallic, Roughness, AO, Specular — pack into 4 channels)
	create_render_target(DXGI_FORMAT_R8G8B8A8_UNORM,
		&m_deferred_resources.gbuffer_material_texture,
		&m_deferred_resources.gbuffer_material_render_target_view,
		&m_deferred_resources.gbuffer_material_shader_resource_view
	);

	// Emissive (can be HDR if needed)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.gbuffer_emissive_texture,
		&m_deferred_resources.gbuffer_emissive_render_target_view,
		&m_deferred_resources.gbuffer_emissive_shader_resource_view
	);

	// Light accumulation buffer (HDR format)
	create_render_target(DXGI_FORMAT_R16G16B16A16_FLOAT,
		&m_deferred_resources.light_texture,
		&m_deferred_resources.light_render_target_view,
		&m_deferred_resources.light_shader_resource_view
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
	func(&m_deferred_resources.light_texture, &m_deferred_resources.light_render_target_view,&m_deferred_resources.light_shader_resource_view);
}

void Renderer::clear_depth(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::clear_emissive(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_window->get_window_info()->emissive_buffer_view, m_clear_emissive_color);
}

void Renderer::clear_full_quad(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearRenderTargetView(m_quad_RTV, m_clear_emissive_color);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::compile_shader(std::string path){

}
