#include <render/renderer.h>
#include <d3dcompiler.h>

#include <filesystem>

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

Renderer::Renderer() : m_shader_files(){
	m_isInitialized = false;
	m_pVBuffer = nullptr;
}

Renderer::Renderer(Renderer&&){

}

Renderer::~Renderer(){

}

bool Renderer::init_pipeline(Engine* engine){

	m_engine_ptr = engine;

	bool ret;

	ID3DBlob* VS = nullptr;
	ID3DBlob* PS = nullptr;
	ID3DBlob* error_msg = nullptr;
	
	if (!std::filesystem::exists("data/shaders/forward/vs_directional.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/vs_directional.hlsl");
	}
	
	if (!std::filesystem::exists("data/shaders/forward/ps_directional.hlsl")) {
		printf("ERROR: Shader file not found: data/shaders/forward/ps_directional.hlsl");
	}

	HRESULT hr = D3DCompileFromFile(L"data/shaders/forward/vs_directional.hlsl", nullptr, nullptr, "VShader", "vs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &VS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;

	hr = D3DCompileFromFile(L"data/shaders/forward/ps_directional.hlsl", nullptr, nullptr, "PShader", "ps_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &PS, &error_msg);
	m_isInitialized = CheckShaderError(hr, error_msg);
	if (!m_isInitialized)return m_isInitialized;

	engine->get_engine_props()->deviceInterface->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(),NULL, &m_shader_files.VS_directional);
	engine->get_engine_props()->deviceInterface->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(),NULL, &m_shader_files.PS_directional);

	//D3DX11CompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
	//D3DX11CompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);
	active_shader(ShaderType::DirectionalLight);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
	bd.ByteWidth = sizeof(Vertex) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		// Using as Vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer
	engine->get_engine_props()->deviceInterface->CreateBuffer(&bd, NULL, &m_pVBuffer);

	// Flags table
	/*
		Flag						CPU Access	GPU Access
		D3D11_USAGE_DEFAULT			None		Read/Write
		DXD11_USAGE_IMMUTABLE		None		Read Only
		DXD11_USAGE_DYNAMIC			Write Only	Read Only
		DXD11_USAGE_STAGING			Read/Write	Read/Write
	*/

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * (3 * 1), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * (3 * 2) , D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	engine->get_engine_props()->deviceInterface->CreateInputLayout(ied,3,VS->GetBufferPointer(), VS->GetBufferSize(), &m_pLayout);
	engine->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout);

	

	upload_triangle();

	return m_isInitialized;
}

void Renderer::active_shader(ShaderType type){
	switch (type){
	case ShaderType::DirectionalLight:
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_directional, nullptr, 0);
		m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_directional, nullptr, 0);
	break;
	case ShaderType::SpotLight:
		break;
	case ShaderType::PointLight:
		break;
	default:break;
	}
}

void Renderer::upload_triangle(){
	assert(m_isInitialized);

	Vertex triangleUP{0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangleLeft{-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangleRight{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	Vertex triangle[3] = {triangleUP, triangleLeft, triangleRight};

	D3D11_MAPPED_SUBRESOURCE ms;
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


}

void Renderer::draw_triangle(){

	//active_shader(ShaderType::DirectionalLight);

	m_engine_ptr->get_engine_props()->inmediateDeviceContext->VSSetShader(m_shader_files.VS_directional, nullptr, 0);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->PSSetShader(m_shader_files.PS_directional, nullptr, 0);

	//printf("VS active %p\n PS active %p\n", m_shader_files.VS_directional, m_shader_files.PS_directional);
	//fflush(stdout);
	//m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetInputLayout(m_pLayout); //debug
	
	// Select wich vertex buffer to display
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetVertexBuffers(0, 1, &m_pVBuffer, &stride, &offset);

	// select wich primitive we are using
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_engine_ptr->get_engine_props()->inmediateDeviceContext->Draw(3,0);

}



void Renderer::release(){
	m_shader_files.VS_directional->Release();
	m_shader_files.PS_directional->Release();
}

void Renderer::compile_shader(std::string path){

}
