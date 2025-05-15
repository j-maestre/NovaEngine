#include "render/imgui/imgui_manager.h"


ImguiManager::ImguiManager() : m_viewport(nullptr){}

ImguiManager::ImguiManager(const ImguiManager&){

}

ImguiManager::ImguiManager(ImguiManager&&)
{
}

ImguiManager::~ImguiManager(){
#ifdef ENABLE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif 
}

void ImguiManager::begin_frame(){
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_viewport = ImGui::GetMainViewport();

	// Llamar al menu principal, que contiene el dockspace
}

void ImguiManager::end_frame(){
	render();
	ImGui::EndFrame();
}

void ImguiManager::render() {

	ImGuiIO& io = ImGui::GetIO();
	
	main_menu();

	// Tu ventana secundaria s� puede estar aqu�
	ImGui::Begin("Example");
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	static float my_color[4];
	ImGui::ColorEdit4("Color", my_color);
	ImGui::End();

	// DEMO
	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);

	// Render
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ImguiManager::resize(HWND hwnd,unsigned int width, unsigned int height){
	ImGuiIO& io = ImGui::GetIO();
	RECT rect;
	GetClientRect(hwnd, &rect);
	//io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	io.DisplaySize = ImVec2(width, height);


	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void ImguiManager::show_demo_window(){


	
}

void ImguiManager::init(HWND handle){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplWin32_Init(handle);
	ImGui_ImplDX11_Init(Engine::get_instance()->get_engine_props()->deviceInterface, Engine::get_instance()->get_engine_props()->inmediateDeviceContext);
}

void ImguiManager::main_menu(){
	ImGui::SetNextWindowSize(ImVec2(m_viewport->Size.x, m_viewport->Size.y));
	ImGui::SetNextWindowPos(ImVec2(m_viewport->Pos.x, m_viewport->Pos.y));
	ImGui::SetNextWindowViewport(m_viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::Begin("main-window", nullptr, window_flags);

	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	//ImGuiDockNodeFlags_NoDockingInCentralNode add this
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode); //|ImGuiDockNodeFlags_AutoHideTabBar  | ImGuiDockNodeFlags_NoResize


	// Menu buttons
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
	if (ImGui::BeginMenuBar()) {
		if(ImGui::MenuItem("Label", "shortcut")){
			
		}
		
		ImGui::EndMenuBar();
	}
	ImGui::PopStyleVar();

	ImGui::End();
}



