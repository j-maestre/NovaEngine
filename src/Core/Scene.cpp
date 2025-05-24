#pragma once
#include "Core/Scene.h"
#include "Core/defines.h"
#include "components/transform_component.h"
#include "components/camera_component.h"
#include "render/imgui/imgui_manager.h"
#include "ryml.hpp"
#include "ryml_std.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>


bool load_transform_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);
bool load_mesh_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);


Scene::Scene(const std::string& path_scene) : m_ecs() {

    m_full_path = "data/scenes/";
    load_scene( { m_full_path + path_scene } );

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

bool Scene::load_scene(std::string path){

    // Leer archivo YAML a string
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error al abrir el archivo: " << path<< "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string yaml_str = buffer.str();

    // Crear árbol de nodos
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_str));

    // Acceder a nodos
    ryml::NodeRef root = tree.rootref();
    ryml::NodeRef entities = root["Entities"];

    bool succes = true;

    for (const ryml::NodeRef& entity : entities.children()) {

        std::string entity_name = std::string(entity.key().data(), entity.key().size());

        Entity e = m_ecs.create_entity(entity_name);
        succes &= load_transform_component(entity, e, m_ecs);
        succes &= load_mesh_component(entity, e, m_ecs);

        if (!succes) {
            ImguiManager::get_instance()->add_resource_loaded("Error loading entity " + entity_name);
        }
    }

    

    return true;
}

Vec3 vec3FromYAML(const ryml::ConstNodeRef& node) {

    auto parseVec3 = [](const ryml::ConstNodeRef& vec) -> Vec3 {
        Vec3 result(0.0f, 0.0f, 0.0f); // Initialize with zeros
        size_t i = 0;

        for (auto&& val : vec.children()) {
            if (i < 3) {
                switch (i){
                case 0: result.x = std::stof(std::string(val.val().data(), val.val().size())); break;
                case 1: result.y = std::stof(std::string(val.val().data(), val.val().size())); break;
                case 2: result.z = std::stof(std::string(val.val().data(), val.val().size())); break;
                }
                ++i;
            }
        }
        return result;
        };

    return parseVec3(node);
}

bool load_transform_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs){

    const auto transform = entity_node["Transform"];
    if (!transform.invalid()) {

        // Extract transform data (position, rotation, scale)
        Vec3 position = vec3FromYAML(transform["Position"]);
        Vec3 rotation = vec3FromYAML(transform["Rotation"]);
        Vec3 scale = vec3FromYAML(transform["Scale"]);

       auto& t = ecs.add_component<TransformComponent>(entity);
       t.set_position(position);
       t.set_rotation(rotation);
       t.set_scale(scale);
    }

    return true;
}

bool load_mesh_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs){

    
    if (entity_node.has_child("MeshComponent")) {

        // Create mesh
        auto& mesh = ecs.add_component<MeshComponent>(entity);

        // Check if is a default model
        std::string model_name(entity_node["MeshComponent"].val().data(), entity_node["MeshComponent"].val().size());
        printf("Loading %s\n", model_name.c_str());

        Model* m = Engine::get_instance()->m_resource.load_mesh(model_name);
        mesh.set_model(m);
    }


    return true;
}
