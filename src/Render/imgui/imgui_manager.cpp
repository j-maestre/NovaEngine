#include "render/imgui/imgui_manager.h"

ImguiManager::ImguiManager(){}

ImguiManager::ImguiManager(const ImguiManager&){

}

ImguiManager::ImguiManager(ImguiManager&&)
{
}

ImguiManager::~ImguiManager(){
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiManager::begin_frame(){
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void ImguiManager::end_frame(){
	ImGui::EndFrame();
	render();
}

void ImguiManager::render() {

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImguiManager::show_demo_window(){

	if (ImGui::GetIO().WantCaptureMouse) {
		// ImGui está capturando el ratón
		//printf("ImGui esta capturando el raton.\n");
	}

	for (int i = 0; i < 5; i++) {

	if (ImGui::GetIO().MouseClicked[i] != 0) {
		printf("Clickardo %d\n",i);
	}
	}
	ImGui::ShowDemoWindow();
}

void ImguiManager::init(HWND handle){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplWin32_Init(handle);
	ImGui_ImplDX11_Init(Engine::get_instance()->get_engine_props()->deviceInterface, Engine::get_instance()->get_engine_props()->inmediateDeviceContext);
}
