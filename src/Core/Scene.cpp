#pragma once
#include "Core/Scene.h"
#include "Core/defines.h"
#include "components/transform_component.h"
#include "components/camera_component.h"
#include "render/imgui/imgui_manager.h"
#include "ryml.hpp"
#include "ryml_std.hpp"
#include <chrono>

Scene::Scene() : m_ecs(){
	const char* yaml_str = R"(
        player:
          name: Xema
          level: 42
          inventory:
            - sword
            - shield
            - potion
    )";

    // Crear árbol de nodos
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_str));

    // Acceder a nodos
    ryml::NodeRef root = tree.rootref();
    ryml::NodeRef player = root["player"];

    std::string name;
    int level;

    player["name"] >> name;
    player["level"] >> level;

    printf("*** Test YAML *** \n");
    std::cout << "Jugador: " << name << "\n";
    std::cout << "Nivel: " << level << "\n";
    std::cout << "Inventario:\n";

    ryml::NodeRef inventory = player["inventory"];
    for (ryml::NodeRef item : inventory.children())
    {
        std::string item_str;
        item >> item_str;
        std::cout << " - " << item_str << "\n";
    }

}

Scene::Scene(const Scene&)
{
}

Scene::Scene(Scene&&)
{
}

Scene::~Scene()
{
}

void Scene::update(){

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif
	auto transforms = m_ecs.viewComponents<TransformComponent>();
	
	for (auto [entity, trans] : transforms.each()) {
		trans.update();
	}

#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;

	ImguiManager::get_instance()->m_update_transform_time = std::chrono::duration<float>(elapsed).count();
#endif
}
