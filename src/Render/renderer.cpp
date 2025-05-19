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
}

Renderer::Renderer(Renderer&&){

}

Renderer::~Renderer(){
	m_depth_buffer->Release();
	m_depth_stencil_view->Release();
	m_cube_index_buffer->Release();
	m_pVBufferConstantCamera->Release();
	m_pVBuffer->Release();
	m_pVBufferCube->Release();
	m_pLayout->Release();
	m_sampler_state->Release();
	m_depth_stencil_state->Release();
}

bool Renderer::init_pipeline(Window* win){

	m_engine_ptr = Engine::get_instance();
	m_window = win;

	bool ret;

	ID3DBlob* VS = nullptr;
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

	HRESULT hr = D3DCompileFromFile(L"data/shaders/forward/vs_common.hlsl", nullptr, nullptr, "VShader", "vs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),NULL, &m_shader_files.VS_common);


	hr = D3DCompileFromFile(L"data/shaders/forward/ps_directional.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(),NULL, &m_shader_files.PS_directional);
	
	hr = D3DCompileFromFile(L"data/shaders/forward/ps_point.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(),NULL, &m_shader_files.PS_point);
	
	hr = D3DCompileFromFile(L"data/shaders/forward/ps_spot.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;
	m_engine_ptr->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(),NULL, &m_shader_files.PS_spot);


	active_shader(ShaderType::DirectionalLight);


	/**** Buffers creation ****/
	ZeroMemory(&m_buffer_description, sizeof(m_buffer_description));
	m_buffer_description.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
	m_buffer_description.ByteWidth = sizeof(Vertex) * 3;
	m_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// Using as Vertex buffer
	m_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_buffer_description, NULL, &m_pVBuffer);

	
	/**** Camera & model constant buffer creation ****/
	ZeroMemory(&m_cam_constant_buffer, sizeof(m_cam_constant_buffer));
	m_cam_constant_buffer.Usage = D3D11_USAGE_DEFAULT;
	m_cam_constant_buffer.ByteWidth = sizeof(CameraConstantBuffer);
	m_cam_constant_buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_cam_constant_buffer.CPUAccessFlags = 0;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cam_constant_buffer,NULL, &m_pVBufferConstantCamera);


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
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,				// TODO: Igual con multiples lights hay que cambiarlo
		//.DepthFunc = D3D11_COMPARISON_LESS,				
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
			.DestBlend = D3D11_BLEND_ONE,
			.BlendOp = D3D11_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D11_BLEND_ONE,
			.DestBlendAlpha = D3D11_BLEND_ONE,
			.BlendOpAlpha = D3D11_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
		}
	  },
	};
	
	m_blend_state_additive = nullptr;
	hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBlendState(&blend_additive_desc, &m_blend_state_additive);
	CheckShaderError(hr, error_msg);
	


	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, m_blend_factor, m_blend_mask);


	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * (3 * 1), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * (3 * 2) , D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	m_engine_ptr->get_engine_props()->deviceInterface->CreateInputLayout(ied,3,VS->GetBufferPointer(), VS->GetBufferSize(), &m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);

	m_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	m_sampler_desc.MinLOD = 0;
	m_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_sampler_state = nullptr;
	m_engine_ptr->get_engine_props()->deviceInterface->CreateSamplerState(&m_sampler_desc, &m_sampler_state);
	

	upload_triangle();

	win->m_renderer = this;

	return m_isInitialized;
}

void Renderer::active_shader(ShaderType type){
	switch (type){
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
	default:break;
	}
}

void Renderer::render_forward(EntityComponentSystem& ecs){

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif

	clear_depth();

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

	//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	
	active_shader(ShaderType::DirectionalLight);
	
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_overwrite, nullptr, 0xffffffff);
	for (auto [entity, directional] : directional_light.each()) {


		if (!directional.get_enabled()) continue;

		directional.update();
		directional.upload_data();

		for (auto [entity, trans, mesh] : transforms.each()) {
			for (Mesh& m : mesh.get_model()->meshes) {
				//__debugbreak();
				render_mesh_internal(cam_buffer, trans, m);
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
				render_mesh_internal(cam_buffer, trans, m);
			}
		}
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetBlendState(m_blend_state_additive, nullptr, 0xffffffff);
	}



	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;

#ifdef ENABLE_IMGUI
	ImguiManager::get_instance()->m_draw_time = std::chrono::duration<float>(elapsed).count();
	ImguiManager::get_instance()->render();
	ImguiManager::get_instance()->scene_info(ecs);
	ImguiManager::get_instance()->show_cam(m_cam, 0xfff);
#endif

}

void Renderer::set_cull_mode(){

}

void Renderer::upload_triangle(){
	assert(m_isInitialized);

	/* Triangle */
	D3D11_MAPPED_SUBRESOURCE ms;
	

	Vertex triangleUP{0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangleLeft{-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangleRight{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangle[3] = {triangleUP, triangleLeft, triangleRight};

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Map(m_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, triangle, sizeof(triangle));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Unmap(m_pVBuffer, NULL);

	// MAP flags
	/*
		D3D11_MAP_READ					Buffer can only be read by the CPU.
		DXD11_MAP_WRITE					Buffer can only be written to by the CPU.
		DXD11_MAP_READ_WRITE			Buffer can be read and written to by the CPU.
		DXD11_MAP_WRITE_DISCARD			Previous contents of buffer are erased, and new buffer is opened for writing.
		DXD11_MAP_WRITE_NO_OVERWRITE	An advanced flag that allows you to add more data to the buffer even while the GPU is using parts.However, you must not work with the parts the GPU is using.
	*/


	/* Cube */
	D3D11_MAPPED_SUBRESOURCE ms_cube;
	//m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_buffer_description, NULL, &m_pVBufferCube);

	Vertex cube[36];

	constexpr float cubePositions[] = {
		// back face
	   -1.0f, -1.0f, -1.0f,  // bottom-left
		1.0f,  1.0f, -1.0f,   // top-right
		1.0f, -1.0f, -1.0f,   // bottom-right         
		1.0f,  1.0f, -1.0f,   // top-right
	   -1.0f, -1.0f, -1.0f,   // bottom-left
	   -1.0f,  1.0f, -1.0f,   // top-left
	   // front face
	   -1.0f, -1.0f,  1.0f,   // bottom-left
		1.0f, -1.0f,  1.0f,   // bottom-right
		1.0f,  1.0f,  1.0f,   // top-right
		1.0f,  1.0f,  1.0f,   // top-right
	   -1.0f,  1.0f,  1.0f,   // top-left
	   -1.0f, -1.0f,  1.0f,   // bottom-left
	   // left face
	   -1.0f,  1.0f,  1.0f,  // top-right
	   -1.0f,  1.0f, -1.0f,  // top-left
	   -1.0f, -1.0f, -1.0f,  // bottom-left
	   -1.0f, -1.0f, -1.0f,  // bottom-left
	   -1.0f, -1.0f,  1.0f,  // bottom-right
	   -1.0f,  1.0f,  1.0f,  // top-right
	   // right face
		1.0f,  1.0f,  1.0f,  // top-left
		1.0f, -1.0f, -1.0f,   // bottom-right
		1.0f,  1.0f, -1.0f,   // top-right         
		1.0f, -1.0f, -1.0f,   // bottom-right
		1.0f,  1.0f,  1.0f,   // top-left
		1.0f, -1.0f,  1.0f,   // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,   // top-right
		 1.0f, -1.0f, -1.0f,   // top-left
		 1.0f, -1.0f,  1.0f,  // bottom-left
		 1.0f, -1.0f,  1.0f,   // bottom-left
		-1.0f, -1.0f,  1.0f,  // bottom-right
		-1.0f, -1.0f, -1.0f,   // top-right
		// top face
		-1.0f,  1.0f, -1.0f,   // top-left
		 1.0f,  1.0f , 1.0f,   // bottom-right
		 1.0f,  1.0f, -1.0f,   // top-right     
		 1.0f,  1.0f,  1.0f,   // bottom-right
		-1.0f,  1.0f, -1.0f,   // top-left
		-1.0f,  1.0f,  1.0f,   // bottom-left  
	};

	constexpr float cubeNormals[] = {
		// back face
		0.0f,  0.0f, -1.0f, // bottom-left
		0.0f,  0.0f, -1.0f,  // top-right
		0.0f,  0.0f, -1.0f,  // bottom-right         
		0.0f,  0.0f, -1.0f,  // top-right
		0.0f,  0.0f, -1.0f,  // bottom-left
		0.0f,  0.0f, -1.0f, // top-left
		// front face
		0.0f,  0.0f,  1.0f,  // bottom-left
		0.0f,  0.0f,  1.0f,  // bottom-right
		0.0f,  0.0f,  1.0f,  // top-right
		0.0f,  0.0f,  1.0f, // top-right
		0.0f,  0.0f,  1.0f,  // top-left
		0.0f,  0.0f,  1.0f,  // bottom-left
		// left face
		-1.0f,  0.0f,  0.0f,  // top-right
		-1.0f,  0.0f,  0.0f, // top-left
		-1.0f,  0.0f,  0.0f,  // bottom-left
		-1.0f,  0.0f,  0.0f,  // bottom-left
		-1.0f,  0.0f,  0.0f, // bottom-right
		-1.0f,  0.0f,  0.0f,  // top-right
		// right face
		1.0f,  0.0f,  0.0f,  // top-left
		1.0f,  0.0f,  0.0f,  // bottom-right
		1.0f,  0.0f,  0.0f,  // top-right         
		1.0f,  0.0f,  0.0f, // bottom-right
		1.0f,  0.0f,  0.0f,  // top-left
		1.0f,  0.0f,  0.0f,  // bottom-left     
		// bottom face
		0.0f, -1.0f,  0.0f,  // top-right
		0.0f, -1.0f,  0.0f,  // top-left
		0.0f, -1.0f,  0.0f,  // bottom-left
		0.0f, -1.0f,  0.0f,  // bottom-left
		0.0f, -1.0f,  0.0f,  // bottom-right
		0.0f, -1.0f,  0.0f,  // top-right
		// top face
		0.0f,  1.0f,  0.0f,  // top-left
		0.0f,  1.0f,  0.0f,  // bottom-right
		0.0f,  1.0f,  0.0f,  // top-right     
		0.0f,  1.0f,  0.0f,  // bottom-right
		0.0f,  1.0f,  0.0f, // top-left
		0.0f,  1.0f,  0.0f,  // bottom-left  
	};

	constexpr float cubeTextureCoords[] = {
		// back face
		0.0f, 0.0f, // bottom-left
		1.0f, 1.0f, // top-right
		1.0f, 0.0f, // bottom-right         
		1.0f, 1.0f, // top-right
		0.0f, 0.0f, // bottom-left
		0.0f, 1.0f, // top-left
		// front face
		0.0f, 0.0f, // bottom-left
		1.0f, 0.0f, // bottom-right
		1.0f, 1.0f, // top-right
		1.0f, 1.0f, // top-right
		0.0f, 1.0f, // top-left
		0.0f, 0.0f, // bottom-left
		// left face
		1.0f, 0.0f, // top-right
		1.0f, 1.0f, // top-left
		0.0f, 1.0f, // bottom-left
		0.0f, 1.0f, // bottom-left
		0.0f, 0.0f, // bottom-right
		1.0f, 0.0f, // top-right
		// right face
		1.0f, 0.0f, // top-left
		0.0f, 1.0f, // bottom-right
		1.0f, 1.0f, // top-right         
		0.0f, 1.0f, // bottom-right
		1.0f, 0.0f, // top-left
		0.0f, 0.0f, // bottom-left     
		// bottom face
		0.0f, 1.0f, // top-right
		1.0f, 1.0f, // top-left
		1.0f, 0.0f, // bottom-left
		1.0f, 0.0f, // bottom-left
		0.0f, 0.0f, // bottom-right
		0.0f, 1.0f, // top-right
		// top face
		0.0f, 1.0f, // top-left
		1.0f, 0.0f, // bottom-right
		1.0f, 1.0f, // top-right     
		1.0f, 0.0f, // bottom-right
		0.0f, 1.0f, // top-left
		0.0f, 0.0f  // bottom-left  
	};


	uint32_t indices[] =
	{
		// back face
		0, 1, 2,
		3, 4, 5,
		// front face
		6, 7, 8,
		9, 10, 11,
		// left face
		12, 13, 14,
		15, 16, 17,
		// right face
		18, 19, 20,
		21, 22, 23,
		// bottom face
		24, 25, 26,
		27, 28, 29,
		// top face
		30, 31, 32,
		33, 34, 35
	};

	for (int i = 0; i < 36; i++) {
		cube[i] = {
			cubePositions[i * 3 + 0],
			cubePositions[i * 3 + 1],
			cubePositions[i * 3 + 2],
			cubeNormals[i * 3 + 0],
			cubeNormals[i * 3 + 1],
			cubeNormals[i * 3 + 2],
			cubeTextureCoords[i * 2 + 0],
			cubeTextureCoords[i * 2 + 1]
		};

	}

	ZeroMemory(&m_buffer_description_cube, sizeof(m_buffer_description_cube));
	m_buffer_description_cube.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
	m_buffer_description_cube.ByteWidth = sizeof(cube);
	m_buffer_description_cube.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// Using as Vertex buffer
	m_buffer_description_cube.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer
	m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_buffer_description_cube, NULL, &m_pVBufferCube);

	// cube index buffer
	ZeroMemory(&m_cube_index_buffer_desc, sizeof(m_cube_index_buffer_desc));
	m_cube_index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	m_cube_index_buffer_desc.ByteWidth = sizeof(indices);
	m_cube_index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_cube_index_buffer_desc.CPUAccessFlags = 0;
	m_cube_index_buffer_desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA init_data = {};
	init_data.pSysMem = indices;

	m_cube_index_buffer = nullptr;
	HRESULT hr = m_engine_ptr->get_engine_props()->deviceInterface->CreateBuffer(&m_cube_index_buffer_desc, &init_data, &m_cube_index_buffer);

	CheckShaderError(hr);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Map(m_pVBufferCube, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms_cube);
	memcpy(ms_cube.pData, cube, sizeof(cube));
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Unmap(m_pVBufferCube, NULL);
	


}

void Renderer::draw_triangle(){

	active_shader(ShaderType::DirectionalLight);

	
	// Select wich vertex buffer to display
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer, &stride, &offset);

	// select wich primitive we are using
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Draw(3,0);

}

void Renderer::draw_cube(){

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

void Renderer::resize(unsigned int width, unsigned int height){
	m_backbuffer_texture->Release();

	//Update depth stencil
	m_depth_buffer->Release();
	m_depth_stencil_view->Release();

	D3D11_TEXTURE2D_DESC depth_desc{
		.Width = width,
		.Height = height,
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
}

void Renderer::render_mesh_internal(CameraConstantBuffer& camera_buffer, TransformComponent& trans, Mesh& m){
	camera_buffer.model = DirectX::XMMatrixTranspose(trans.get_transform());

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->UpdateSubresource(m_pVBufferConstantCamera, 0, nullptr, &camera_buffer, 0, 0);

	// Buffer de la camara y la model del objeto subido, ahora draw cube
	//const std::vector<Vertex> cube = m_engine_ptr->get_cube();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m.buffer, &stride, &offset);



	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetSamplers(0, 1, &m_sampler_state);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShaderResources(0, 1, &(m.material.get_albedo()->m_data.texture_view));
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetIndexBuffer(m.index_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->DrawIndexed(m.num_indices, 0, 0);
}

void Renderer::clear_depth(){
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->OMSetRenderTargets(1, &m_window->get_window_info()->backbuffer, m_depth_stencil_view);
}

void Renderer::compile_shader(std::string path){

}
