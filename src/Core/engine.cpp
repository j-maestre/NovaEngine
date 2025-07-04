#include "Core/engine.h"
#include <Core/defines.h>
#include <Core/input.h>
#include <assert.h>
#include <chrono>
#include <Windows.h>
#include "Core/JobSystem.h"
#include <wrl/client.h>


Engine::Engine() : m_props(std::make_shared<EngineProps>()), m_input(), m_resource(){

	m_resource.set_engine(this);
	m_last_time = std::chrono::high_resolution_clock::now();
	m_time = 0.0f;
}

/*
Engine::Engine(const Engine&){

}
*/

Engine::~Engine(){
	m_raster_state->Release();
}

void Engine::init(Window* window){
	
	assert(window != nullptr);

	//window_used->window_handle

	window->m_input = &m_input;

	// clear out the struct for use
	ZeroMemory(&(m_props->scd), sizeof(DXGI_SWAP_CHAIN_DESC1));

	WindowInfo* window_info = window->get_window_info();

	
#ifdef _DEBUG
	m_props->flags |= D3D11_CREATE_DEVICE_DEBUG;				// Debug
#endif
	//m_props->flags |= D3D11_CREATE_DEVICE_SWITCH_TO_REF;		// Change hardware mode to reference mode
	//m_props->flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;		// Allows Direct2D working with Direct3D
	//m_props->flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;		// Singlethread


	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		m_props->flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&m_props->deviceInterface,
		nullptr,
		&m_props->inmediateDeviceContext
	);
	assert(!FAILED(hr));

	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	hr = m_props->deviceInterface->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	assert(!FAILED(hr));

	ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	assert(!FAILED(hr));

	ComPtr<IDXGIFactory2> dxgiFactory = nullptr;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
	assert(!FAILED(hr));

	m_props->scd.Width = window->get_window_properties()->width;
	m_props->scd.Height = window->get_window_properties()->height;
	m_props->scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_props->scd.Stereo = FALSE;
	m_props->scd.SampleDesc.Count = 1;
	m_props->scd.SampleDesc.Quality = 0;
	m_props->scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_props->scd.BufferCount = NUM_BUFFERING;//NUM_BUFFERING;
	m_props->scd.Scaling = DXGI_SCALING_STRETCH;
	m_props->scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	if (NUM_BUFFERING > 1) {
		m_props->scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}
	m_props->scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//m_props->scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swap_chain_tmp;

	hr = dxgiFactory->CreateSwapChainForHwnd(
		m_props->deviceInterface,
		window_info->window_handle,
		&m_props->scd,
		nullptr,        // fullscreenDesc -> nullptr significa modo ventana
		nullptr,
		&swap_chain_tmp
	);
	assert(!FAILED(hr));
	
	ComPtr<IDXGISwapChain3> tempSwapChain3;
	hr = swap_chain_tmp.As(&tempSwapChain3);
	assert(!FAILED(hr));
	m_props->swapChain = tempSwapChain3;


	assert(!FAILED(hr));
	//swap_chain_tmp->Release();
	//tempSwapChain3->Release();
	printf("SwapChain created with BufferCount=%d, SwapEffect=%d\n", m_props->scd.BufferCount, m_props->scd.SwapEffect);

	// Setting rasterizer
	ZeroMemory(&m_raster, sizeof(D3D11_RASTERIZER_DESC));
	m_raster.CullMode = D3D11_CULL_BACK;    // Desactiva el culling
	m_raster.FillMode = D3D11_FILL_SOLID;   // Rellenar las caras con un color solido
	m_raster.FrontCounterClockwise = FALSE; // La orientacion de las caras frontales no cambia
	m_raster.DepthBias = 0;
	m_raster.SlopeScaledDepthBias = 0.0f;
	m_raster.DepthBiasClamp = 0.0f;
	m_raster.ScissorEnable = FALSE;         // No se usa el scissor test
	m_raster.MultisampleEnable = FALSE;     // No multisampling
	m_raster.AntialiasedLineEnable = FALSE; // No se usan lineas antialiasing
	
	hr = m_props->deviceInterface->CreateRasterizerState(&m_raster, &m_raster_state);
	assert(!FAILED(hr));
	m_props->inmediateDeviceContext->RSSetState(m_raster_state);

	
	

	// Setting rasterizer for skybox
	ZeroMemory(&m_raster_skybox, sizeof(D3D11_RASTERIZER_DESC));
	m_raster_skybox.FillMode = D3D11_FILL_SOLID;  
	m_raster_skybox.CullMode = D3D11_CULL_NONE;
	m_raster_skybox.DepthClipEnable = true;
	//m_raster_skybox.FrontCounterClockwise = TRUE;

	hr = m_props->deviceInterface->CreateRasterizerState(&m_raster_skybox, &m_raster_state_skybox);
	assert(!FAILED(hr));
	

	// Setting the viewport
	WindowProperties* win_props = window->get_window_properties();
	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = (float) win_props->width; // SCREEN_WIDTH
	m_viewport.Height = (float) win_props->height; // SCREEN_HEIGHT
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_props->inmediateDeviceContext->RSSetViewports(1, &m_viewport);
	window->m_swapChain = m_props->swapChain;
	window->m_deviceInterface = m_props->deviceInterface;
	window->m_inmediateDeviceContext = m_props->inmediateDeviceContext;

	//unsigned int count = 0;
	//m_props->swapChain->GetLastPresentCount(&count);
	//m_props->swapChain->Present(0,0);
	//m_props->swapChain->GetLastPresentCount(&count);

}


void Engine::update(){

#ifdef MEASURE_TIME
	auto start = std::chrono::high_resolution_clock::now();
#endif

	//m_props->inmediateDeviceContext->RSSetViewports(1, &m_viewport);

	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = currentTime - m_last_time;
	m_last_time = currentTime;
	m_delta_time = delta.count();
	m_time += m_delta_time;
	
	m_resource.check_models_to_load();
	m_resource.check_textures_to_load();

#ifdef ENABLE_IMGUI
	ImguiManager::get_instance()->m_draw_calls = 0;
#endif

#ifdef MEASURE_TIME
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	ImguiManager::get_instance()->m_engine_update_time = std::chrono::duration<float>(elapsed).count();
#endif
}

void Engine::release(){
	m_props->swapChain->SetFullscreenState(FALSE, NULL);	// TODO: method to change full screen?
	m_props->swapChain->Release();
	m_props->deviceInterface->Release();
	m_props->inmediateDeviceContext->Release();

	for (Scene* scene : m_all_scenes) {
		if (scene != nullptr) delete scene;
	}

}

void Engine::open_console(){
	// Open console
	AllocConsole(); // Crea una nueva consola
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout); // Redirige stdout a la consola
	freopen_s(&file, "CONOUT$", "w", stderr); // Redirige stderr a la consola
	freopen_s(&file, "CONIN$", "r", stdin);   // Redirige stdin a la consola
	printf("\n*** Console opened succesfully ***\n");
}

Scene* Engine::get_scene(){

	return m_current_scene;
}

Scene* Engine::create_scene(std::string path){
	Scene* new_scene = new Scene(path.c_str());
	m_all_scenes.push_back(new_scene);

	return new_scene;
}

void Engine::set_scene(Scene* s){
	m_current_scene = s;
}

float Engine::get_delta_time(){

	return m_delta_time;
}

float Engine::get_fps(){

	return 1.0f / m_delta_time;
}

float Engine::get_time(){

	return m_time;
}

void Engine::load_default_textures(){
	//Texture* t = m_resource.load_texture("data/textures/jou.jpg");
	//Texture* t = m_resource.load_texture("data/textures/default_albedo.png");
	//Texture* t = m_resource.load_texture("data/textures/red.png");
	Texture* t = m_resource.load_texture("data/textures/black.png");
	Texture* t2 = m_resource.load_texture("data/textures/kirby.png");
	Texture* normal = m_resource.load_texture("data/textures/default_normal.png");
	Texture* metallic = m_resource.load_texture("data/textures/default_metallic_new.png");
	Texture* roughness = m_resource.load_texture("data/textures/default_roughness_new.png");
	Texture* ao = m_resource.load_texture("data/textures/default_ao.png");
	Texture* emissive = m_resource.load_texture("data/textures/black.png");

	m_default_texture_albedo = t->get_id();
	m_texture_tmp = t2->get_id();

	m_default_material.m_initialised = true;
	m_default_material.set_texture_albedo(t);
	m_default_material.set_texture_normal(normal);
	m_default_material.set_texture_metallic(metallic);
	m_default_material.set_texture_roughness(roughness);
	//m_default_material.set_texture_roughness(metallic);		// just for debug
	m_default_material.set_texture_ao(ao);
	m_default_material.set_texture_emissive(emissive);

	m_default_material.set_metallic_value(0.5f);
	m_default_material.set_roughness_value(0.5f);

	//m_resource.load_cube_map("data/textures/sky.dds");
	m_resource.load_cube_map("data/textures/cubemap/pruebaEnvHDR.dds");
	// m_resource.load_cube_map("data/textures/clouds.dds");

	m_resource.check_textures_to_load();
}

Texture* Engine::get_default_albedo_texture(){

	return m_resource.get_texture(m_default_texture_albedo);
}

#define MULTI_THREAD
void Engine::init_geometries(){

	load_default_textures();

	auto start = std::chrono::high_resolution_clock::now();
	m_resource.load_mesh("data/models/basics/cube.fbx");
	m_cube_model = m_resource.load_mesh("data/models/basics/cube.fbx");
#ifdef MULTI_THREAD

	std::vector<std::function<void()>> tasks;

	auto task2 = [this]() {
		this->m_resource.load_mesh("data/models/basics/cylinder.fbx", true);
	};	
	auto task3 = [this]() {
		this->m_resource.load_mesh("data/models/basics/cylinder_high.fbx", true);
	};

	auto task4 = [this]() {
		this->m_resource.load_mesh("data/models/basics/cone.fbx", true);
	};
	
	auto task5 = [this]() {
		this->m_resource.load_mesh("data/models/basics/cone_high.fbx", true);
	};

	auto task6 = [this]() {
		this->m_resource.load_mesh("data/models/basics/sphere.fbx", true);
	};
	
	auto task7 = [this]() {
		this->m_resource.load_mesh("data/models/basics/sphere_medium_resolution.fbx", true);
	};
	
	auto task8 = [this]() {
		this->m_resource.load_mesh("data/models/basics/sphere_high_resolution.fbx", true);
	};

	//tasks.push_back(task1);
	tasks.push_back(task2);
	tasks.push_back(task3);
	tasks.push_back(task4);
	tasks.push_back(task5);
	tasks.push_back(task6);
	tasks.push_back(task7);
	tasks.push_back(task8);

	m_resource.m_job_system.add_task(tasks);
	m_resource.m_job_system.wait_until_finish();
	m_resource.check_models_to_load();

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	float load_meshes_time = std::chrono::duration<float>(elapsed).count();
	printf("Load default meshes time multithread %f\n", load_meshes_time);

#endif

	//m_cube_model =  m_resource.load_mesh("data/models/basics/cube.fbx");
	m_cylinder_model =  m_resource.load_mesh("data/models/basics/cylinder.fbx");
	m_cylinder_high_model =  m_resource.load_mesh("data/models/basics/cylinder_high.fbx");
	m_cone_model =  m_resource.load_mesh("data/models/basics/cone.fbx");
	m_cone_high_model =  m_resource.load_mesh("data/models/basics/cone_high.fbx");
	m_sphere_model = m_resource.load_mesh("data/models/basics/sphere.fbx");
	m_sphere_medium_model = m_resource.load_mesh("data/models/basics/sphere_medium_resolution.fbx");
	m_sphere_high_model = m_resource.load_mesh("data/models/basics/sphere_high_resolution.fbx");

#ifndef MULTI_THREAD
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;
	float load_meshes_time = std::chrono::duration<float>(elapsed).count();
	printf("Load default meshes time single thread %f\n", load_meshes_time);
#endif

	
	/*
	*/
	std::vector<Model*> tmp = { m_cube_model , m_cylinder_model, m_cylinder_high_model,m_cone_model,m_cone_high_model,m_sphere_model,m_sphere_medium_model,m_sphere_high_model };
	
	for (auto& model : tmp) {

		for (auto& mesh : model->meshes) {
			
			mesh.material = m_default_material;
		}
	}

}
