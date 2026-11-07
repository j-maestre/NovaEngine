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
#include <cstdio>
#include <algorithm>
#include <unordered_map>


bool load_transform_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);
Vec3 vec3FromYAML(const ryml::ConstNodeRef& node);
template <typename T> T get_value_from_yaml(c4::yml::ConstNodeRef entity_node);
static std::string format_float(float value);
static std::string format_vec3(const Vec3& v);
bool load_mesh_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);
bool load_material_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);
bool load_light_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs);


Scene::Scene(const std::string& path_scene) : m_ecs(), m_selected_entity(-1,"") {

    m_full_path = "data/scenes/";
    m_path = m_full_path + path_scene;
    load_scene(m_path);
    reset_history();

}

Scene::Scene(const Scene&) : m_selected_entity(-1,"") {
    
}

Scene::Scene(Scene&&) : m_selected_entity(-1, ""){

}

Scene::~Scene()
{
}

void Scene::update(){

	// Los componentes de malla no se pueden destruir mientras hay cargas que los apuntan
	if (!m_commands.empty() && !Engine::get_instance()->m_resource.is_loading_models()) {
		std::vector<std::function<void()>> commands;
		commands.swap(m_commands);
		for (auto& command : commands) {
			command();
		}
	}

	if (m_history_check && m_commands.empty()) {
		m_history_check = false;
		record_history();
	}

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif
	auto transforms = m_ecs.viewComponents<TransformComponent>();
	
	for (auto [entity, trans] : transforms.each()) {
		trans.update();
	}

	sync_light_transforms();

#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;

	ImguiManager::get_instance()->m_update_transform_time = std::chrono::duration<float, std::milli>(elapsed).count();
#endif
}

Entity Scene::get_selected_entity(){

    return m_selected_entity;
}

void Scene::select_entity(Entity e){

    m_selected_entity = e;
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
    m_environment = SceneEnvironment();
    const std::string text = buffer.str();
    load_text(text);

    // Solo al abrir el archivo: los estados del historial no la incluyen, para que deshacer no mueva la camara
    CameraComponent* camera = Engine::get_instance()->get_camera();
    if (camera) {
        std::string yaml_copy = text;
        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_copy));
        ryml::ConstNodeRef root = tree.rootref();
        if (root.has_child("Camera")) {
            ryml::ConstNodeRef node = root["Camera"];
            if (node.has_child("Position")) camera->set_position(vec3FromYAML(node["Position"]));
            if (node.has_child("Pitch")) camera->set_pitch(get_value_from_yaml<float>(node["Pitch"]));
            if (node.has_child("Yaw")) camera->set_yaw(get_value_from_yaml<float>(node["Yaw"]));
        }
    }
    Engine::get_instance()->m_resource.set_environment(m_environment.skybox, m_environment.irradiance, m_environment.specular);

    return true;
}

void Scene::load_text(const std::string& text){

    std::string yaml_str = text;

    // Crear arbol de nodos
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_str));

    // Acceder a nodos
    ryml::NodeRef root = tree.rootref();
    ryml::NodeRef entities = root["Entities"];

    if (root.has_child("Environment")) {
        const auto read_path = [&](const char* key, std::string& path) {
            const ryml::csubstr name = ryml::to_csubstr(key);
            if (root["Environment"].has_child(name)) {
                const ryml::csubstr value = root["Environment"][name].val();
                path = std::string(value.data(), value.size());
            }
        };
        read_path("Skybox", m_environment.skybox);
        read_path("Irradiance", m_environment.irradiance);
        read_path("Specular", m_environment.specular);
    }

    bool succes = true;

    for (const ryml::NodeRef& entity : entities.children()) {

        std::string entity_name = std::string(entity.key().data(), entity.key().size());

        Entity e = m_ecs.create_entity(entity_name);
        succes &= load_transform_component(entity, e, m_ecs);
        succes &= load_mesh_component(entity, e, m_ecs);
        succes &= load_material_component(entity, e, m_ecs);
        succes &= load_light_component(entity, e, m_ecs);

        if (!succes) {
            ImguiManager::get_instance()->add_resource_loaded("Error loading entity " + entity_name);
        }
    }
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

template <typename T>
T get_value_from_yaml(c4::yml::ConstNodeRef entity_node) {
    T value = std::stof(std::string(entity_node.val().data(), entity_node.val().size()));
    return value;
}

template <>
bool get_value_from_yaml(c4::yml::ConstNodeRef entity_node) {
    ryml::csubstr val = entity_node.val();
    if (val == "true" || val == "True" || val == "1") return true;
    
    return false;
}


bool load_transform_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs){

    if (entity_node.has_child("Transform")) {

        const auto transform = entity_node["Transform"];
        if (!transform.invalid()) {

            // Extract transform data (position, rotation, scale)
            Vec3 position = vec3FromYAML(transform["Position"]);
            Vec3 rotation = vec3FromYAML(transform["Rotation"]);
            Vec3 scale = vec3FromYAML(transform["Scale"]);


            rotation.x = degToRad(rotation.x);
            rotation.y = degToRad(rotation.y);
            rotation.z = degToRad(rotation.z);

           auto& t = ecs.add_component<TransformComponent>(entity);
           t.set_position(position);
           t.set_rotation(rotation);
           t.set_scale(scale);

           if (transform.has_child("Parent")) {
               std::string model_name(transform["Parent"].val().data(), transform["Parent"].val().size());
               Entity e = ecs.get_entity_by_name(model_name);
               if (e.get_id() != -1) {
                   TransformComponent* trans = ecs.get_component<TransformComponent>(e);
                   t.set_parent(trans);
               }
           }
        }
    }

    return true;
}

bool load_mesh_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs){

    
    if (entity_node.has_child("MeshComponent")) {

        // Create mesh
        ecs.add_component<MeshComponent>(entity);
        MeshComponent* mesh = ecs.get_component<MeshComponent>(entity);

        // Check if is a default model
        std::string model_name(entity_node["MeshComponent"].val().data(), entity_node["MeshComponent"].val().size());
        printf("Loading %s\n", model_name.c_str());
        mesh->set_model_path(model_name);

        Engine::get_instance()->m_resource.load_mesh_async(model_name, mesh);
        //Model* m = Engine::get_instance()->m_resource.load_mesh(model_name);
        //mesh->set_model(m);
    }


    return true;
}

static Material parse_material(ryml::ConstNodeRef properties){

    Material material;
    material.init_material();

    for (auto&& t : properties.children()) {
        if (t.has_child("Texture")) {
            std::string textureName(t["Texture"].val().data(), t["Texture"].val().size());
            std::string type(t["Type"].val().data(), t["Type"].val().size());

            // El color va en sRGB; normales, metallic, roughness y AO son datos lineales
            const bool is_color = type == "Albedo" || type == "Emissive";
            ResourceManager& resources = Engine::get_instance()->m_resource;
            Texture* t_loaded = is_color ? resources.load_texture_srgb(textureName) : resources.load_texture(textureName);

            if (type == "Albedo") material.set_texture_albedo(t_loaded);
            if (type == "Normal") material.set_texture_normal(t_loaded);
            if (type == "Metallic") material.set_texture_metallic(t_loaded);
            if (type == "Roughness") material.set_texture_roughness(t_loaded);
            if (type == "AO") material.set_texture_ao(t_loaded);
            if (type == "Emissive") material.set_texture_emissive(t_loaded);

        }else {
            if (t.has_child("Metallic")) material.set_metallic_value(get_value_from_yaml<float>(t["Metallic"]));
            if (t.has_child("Roughness")) material.set_roughness_value(get_value_from_yaml<float>(t["Roughness"]));

            if (t.has_child("Albedo")) {
                Vec3 c = vec3FromYAML(t["Albedo"]);
                material.set_color_value({ c.x, c.y, c.z, 1.0f });
            }

            if (t.has_child("EmissiveIntensity")) material.set_emissive_intensity(get_value_from_yaml<float>(t["EmissiveIntensity"]));
            if (t.has_child("EmissiveBloom")) material.set_emissive_bloom(get_value_from_yaml<float>(t["EmissiveBloom"]));
            if (t.has_child("EmissivePulse")) {
                Vec3 pulse = vec3FromYAML(t["EmissivePulse"]);
                material.set_emissive_pulse(pulse.x, pulse.y);
            }
            if (t.has_child("EmissiveColor")) {
                Vec3 c = vec3FromYAML(t["EmissiveColor"]);
                material.set_emissive_value({ c.x, c.y, c.z });
            }

            if (t.has_child("UVScale")) {
                Vec3 c = vec3FromYAML(t["UVScale"]);
                material.set_uv_scale({ c.x, c.y });
            }
        }
    }

    return material;
}

bool load_material_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs){

    MeshComponent* mesh = ecs.get_component<MeshComponent>(entity);
    if (!mesh) return true;

    // Materials: una lista por cada material del archivo. Material: el mismo para todas las mallas.
    std::vector<Material> materials;
    if (entity_node.has_child("Materials")) {
        for (auto&& slot : entity_node["Materials"].children()) {
            materials.push_back(parse_material(slot));
        }
    }
    else if (entity_node.has_child("Material")) {
        materials.push_back(parse_material(entity_node["Material"]));
    }

    if (!materials.empty()) {
        mesh->set_materials(materials);
    }

    return true;
}

// Todas las luces tienen transform: asi se pueden mover con el gizmo y emparentar. Si el yaml
// no trae uno, se crea con la posicion de la luz.
static void ensure_light_transform(ryml::ConstNodeRef light_component, Entity& entity, EntityComponentSystem& ecs){

    if (ecs.get_component<TransformComponent>(entity)) return;

    Vec3 position = { 0.0f, 0.0f, 0.0f };
    if (light_component.has_child("Position")) position = vec3FromYAML(light_component["Position"]);
    ecs.add_component<TransformComponent>(entity, position);
}

static void read_optional(ryml::ConstNodeRef node, const char* key, const std::function<void(float)>& setter){

    ryml::csubstr name = ryml::to_csubstr(key);
    if (node.has_child(name)) setter(get_value_from_yaml<float>(node[name]));
}

bool load_light_component(const ryml::NodeRef& entity_node, Entity& entity, EntityComponentSystem& ecs) {
    
    if (entity_node.has_child("Light")) {

        const auto light_component = entity_node["Light"];
         
 

        std::string light_type(light_component["Type"].val().data(), light_component["Type"].val().size());
        ensure_light_transform(light_component, entity, ecs);
        
        if (light_type == "Directional") {
            ecs.add_component<DirectionalLight>(entity);
            
            DirectionalLight* light = ecs.get_component<DirectionalLight>(entity);

            Vec3 direction = vec3FromYAML(light_component["Direction"]);
            Vec3 color = vec3FromYAML(light_component["Color"]);
            
            bool enabled = false;
            if (light_component.has_child("Enabled")) {
                enabled = get_value_from_yaml<bool>(light_component["Enabled"]);
            }

            light->set_color(color);
            light->set_direction(direction);
            light->set_enabled(enabled);
            read_optional(light_component, "Intensity", [&](float v) { light->set_intensity(v); });


        }
        
        
        if (light_type == "Point") {
            PointLight& light = ecs.add_component<PointLight>(entity);
            
            // La posicion real viene del transform; en el yaml es opcional
            Vec3 position = { 0.0f, 0.0f, 0.0f };
            if (light_component.has_child("Position")) position = vec3FromYAML(light_component["Position"]);
            Vec3 color = vec3FromYAML(light_component["Color"]);

            float distance = get_value_from_yaml<float>(light_component["Distance"]);
            float intensity = get_value_from_yaml<float>(light_component["Intensity"]);
            float attenuation = get_value_from_yaml<float>(light_component["Attenuation"]);
            float falloff = get_value_from_yaml<float>(light_component["FallOff"]);
            float fallstart = get_value_from_yaml<float>(light_component["FallStart"]);

            bool enabled = false;
            if (light_component.has_child("Enabled")) {
                enabled = get_value_from_yaml<bool>(light_component["Enabled"]);
            }

            light.set_position(position);
            light.set_color(color);
            light.set_distance(distance);
            light.set_intensity(intensity);
            light.set_attenuation(attenuation);
            light.set_fall_off(falloff);
            light.set_fall_start(fallstart);
            light.set_enabled(enabled);
            read_optional(light_component, "Constant", [&](float v) { light.set_constant_attenuation(v); });
            read_optional(light_component, "Linear", [&](float v) { light.set_linear_attenuation(v); });
            read_optional(light_component, "Quadratic", [&](float v) { light.set_quadratic_attenuation(v); });
            if (light_component.has_child("Pulse")) {
                Vec3 pulse = vec3FromYAML(light_component["Pulse"]);
                light.set_pulse(pulse.x, pulse.y);
            }
        }

        if (light_type == "Spot") {
            SpotLight& light = ecs.add_component<SpotLight>(entity);

            // La posicion real viene del transform; en el yaml es opcional
            Vec3 position = { 0.0f, 0.0f, 0.0f };
            if (light_component.has_child("Position")) position = vec3FromYAML(light_component["Position"]);
            Vec3 direction = vec3FromYAML(light_component["Direction"]);
            Vec3 color = vec3FromYAML(light_component["Color"]);

            float distance = get_value_from_yaml<float>(light_component["Distance"]);
            //float intensity = get_value_from_yaml<float>(light_component["Intensity"]);
            //float attenuation = get_value_from_yaml<float>(light_component["Attenuation"]);
            //float falloff = get_value_from_yaml<float>(light_component["FallOff"]);
            //float fallstart = get_value_from_yaml<float>(light_component["FallStart"]);

            bool enabled = false;
            if (light_component.has_child("Enabled")) {
                enabled = get_value_from_yaml<bool>(light_component["Enabled"]);
            }

            light.set_color(color);
            light.set_position(position);
            light.set_direction(direction);
            light.set_enabled(enabled);
            light.set_distance(distance);
            read_optional(light_component, "Intensity", [&](float v) { light.set_intensity(v); });
            read_optional(light_component, "InnerAngle", [&](float v) { light.set_inner_circle(v); });
            read_optional(light_component, "OuterAngle", [&](float v) { light.set_outer_circle(v); });
            read_optional(light_component, "Constant", [&](float v) { light.set_constant_attenuation(v); });
            read_optional(light_component, "Linear", [&](float v) { light.set_linear_attenuation(v); });
            read_optional(light_component, "Quadratic", [&](float v) { light.set_quadratic_attenuation(v); });
            //light.set_distance(distance);
            //light.set_intensity(intensity);
            //light.set_atte(attenuation);
            //light.set_fall_off(falloff);
            //light.set_fall_start(fallstart);
        }

    }

    return true;
}

// ---------------------------------------------------------------------------
// Guardado
// ---------------------------------------------------------------------------

static std::string format_float(float value){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.6g", value);
    return buffer;
}

static std::string format_vec3(const Vec3& v){
    return "[" + format_float(v.x) + ", " + format_float(v.y) + ", " + format_float(v.z) + "]";
}

static std::string yaml_string(const std::string& text){

    std::string out = "\"";
    for (char c : text) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

static void write_material(std::string& out, Material& m, const std::string& first, const std::string& rest){

    ResourceManager& resources = Engine::get_instance()->m_resource;
    std::string lines;

    auto texture = [&](Texture* t, const char* type) {
        const std::string path = resources.get_texture_path(t);
        if (!path.empty()) lines += "{Texture: " + yaml_string(path) + ", Type: \"" + type + "\"}\n";
    };
    texture(m.get_albedo(), "Albedo");
    texture(m.get_normal(), "Normal");
    texture(m.get_metallic(), "Metallic");
    texture(m.get_roughness(), "Roughness");
    texture(m.get_ao(), "AO");
    texture(m.get_emissive(), "Emissive");

    const Vec4 color = m.get_color_value();
    const Vec2 uv = m.get_uv_scale();
    lines += "{Albedo: " + format_vec3({ color.x, color.y, color.z }) + "}\n";
    lines += "{Metallic: " + format_float(m.get_metallic_value()) + "}\n";
    lines += "{Roughness: " + format_float(m.get_roughness_value()) + "}\n";
    lines += "{EmissiveColor: " + format_vec3(m.get_emissive_value()) + "}\n";
    lines += "{EmissiveIntensity: " + format_float(m.get_emissive_intensity()) + "}\n";
    if (m.get_emissive_bloom() != 1.0f) lines += "{EmissiveBloom: " + format_float(m.get_emissive_bloom()) + "}\n";
    if (m.get_emissive_pulse_speed() != 0.0f) lines += "{EmissivePulse: [" + format_float(m.get_emissive_pulse_speed()) + ", " + format_float(m.get_emissive_pulse_min()) + "]}\n";
    lines += "{UVScale: [" + format_float(uv.x) + ", " + format_float(uv.y) + "]}\n";

    std::stringstream stream(lines);
    std::string line;
    bool is_first = true;
    while (std::getline(stream, line)) {
        out += (is_first ? first : rest) + line + "\n";
        is_first = false;
    }
}

std::vector<Entity*> Scene::children_of(TransformComponent* parent){

    std::vector<Entity*> result;
    for (Entity& e : m_ecs.get_entities()) {
        TransformComponent* t = m_ecs.get_component<TransformComponent>(e);
        TransformComponent* entity_parent = t ? t->get_parent() : nullptr;
        if (entity_parent == parent) result.push_back(&e);
    }
    return result;
}

void Scene::write_entity(std::string& out, Entity& e, const std::string& name, const std::string& parent){

    EntityComponentSystem& ecs = m_ecs;
    out += "  " + yaml_string(name) + ":\n";

    if (TransformComponent* t = ecs.get_component<TransformComponent>(e)) {
        const Vec3 rotation = t->get_rotation();
        out += "    Transform:\n";
        out += "      Position: " + format_vec3(t->get_position()) + "\n";
        out += "      Rotation: " + format_vec3({ radToDeg(rotation.x), radToDeg(rotation.y), radToDeg(rotation.z) }) + "\n";
        out += "      Scale: " + format_vec3(t->get_scale()) + "\n";
        if (!parent.empty()) out += "      Parent: " + yaml_string(parent) + "\n";
    }

    if (MeshComponent* mesh = ecs.get_component<MeshComponent>(e)) {
        out += "    MeshComponent: " + yaml_string(mesh->get_model_path()) + "\n";

        if (mesh->has_own_materials()) {
            std::vector<Material>& materials = mesh->get_materials();
            if (materials.size() == 1) {
                out += "    Material:\n";
                write_material(out, materials[0], "      - ", "      - ");
            }
            else if (materials.size() > 1) {
                out += "    Materials:\n";
                for (Material& m : materials) {
                    write_material(out, m, "      - - ", "        - ");
                }
            }
        }
    }

    const auto bool_text = [](bool value) { return value ? "true" : "false"; };

    if (DirectionalLight* l = ecs.get_component<DirectionalLight>(e)) {
        out += "    Light:\n      Type: \"Directional\"\n";
        out += "      Direction: " + format_vec3(l->get_direction()) + "\n";
        out += "      Color: " + format_vec3(l->get_color()) + "\n";
        out += "      Intensity: " + format_float(l->get_intensity()) + "\n";
        out += std::string("      Enabled: ") + bool_text(l->get_enabled()) + "\n";
    }

    if (PointLight* l = ecs.get_component<PointLight>(e)) {
        out += "    Light:\n      Type: \"Point\"\n";
        out += "      Position: " + format_vec3(l->get_position()) + "\n";
        out += "      Color: " + format_vec3(l->get_color()) + "\n";
        out += "      Distance: " + format_float(l->get_distance()) + "\n";
        out += "      Intensity: " + format_float(l->get_intensity()) + "\n";
        out += "      Attenuation: " + format_float(l->get_attenuation()) + "\n";
        out += "      FallOff: " + format_float(l->get_fall_off()) + "\n";
        out += "      FallStart: " + format_float(l->get_fall_start()) + "\n";
        if (l->get_pulse_speed() != 0.0f) out += "      Pulse: [" + format_float(l->get_pulse_speed()) + ", " + format_float(l->get_pulse_min()) + "]\n";
        out += "      Constant: " + format_float(l->get_constant_attenuation()) + "\n";
        out += "      Linear: " + format_float(l->get_linear_attenuation()) + "\n";
        out += "      Quadratic: " + format_float(l->get_quadratic_attenuation()) + "\n";
        out += std::string("      Enabled: ") + bool_text(l->get_enabled() > 0.0f) + "\n";
    }

    if (SpotLight* l = ecs.get_component<SpotLight>(e)) {
        out += "    Light:\n      Type: \"Spot\"\n";
        out += "      Position: " + format_vec3(l->get_position()) + "\n";
        out += "      Direction: " + format_vec3(l->get_direction()) + "\n";
        out += "      Color: " + format_vec3(l->get_color()) + "\n";
        out += "      Distance: " + format_float(l->get_distance()) + "\n";
        out += "      Intensity: " + format_float(l->get_intensity()) + "\n";
        out += "      InnerAngle: " + format_float(l->get_inner_circle()) + "\n";
        out += "      OuterAngle: " + format_float(l->get_outside_circle()) + "\n";
        out += "      Constant: " + format_float(l->get_constant_attenuation()) + "\n";
        out += "      Linear: " + format_float(l->get_linear_attenuation()) + "\n";
        out += "      Quadratic: " + format_float(l->get_quadratic_attenuation()) + "\n";
        out += std::string("      Enabled: ") + bool_text(l->get_enabled() > 0.0f) + "\n";
    }
}

// El padre siempre se escribe antes que sus hijos: al cargar, Parent se resuelve por nombre
void Scene::write_subtree(std::string& out, Entity& e, const std::string& parent, const std::function<std::string(Entity&)>& name_of){

    const std::string name = name_of(e);
    write_entity(out, e, name, parent);

    if (TransformComponent* t = m_ecs.get_component<TransformComponent>(e)) {
        for (Entity* child : children_of(t)) {
            write_subtree(out, *child, name, name_of);
        }
    }
}

std::string Scene::serialize(){

    std::string out;
    const SceneEnvironment default_environment;
    if (m_environment.skybox != default_environment.skybox || m_environment.irradiance != default_environment.irradiance || m_environment.specular != default_environment.specular) {
        out += "Environment:\n";
        out += "  Skybox: \"" + m_environment.skybox + "\"\n";
        out += "  Irradiance: \"" + m_environment.irradiance + "\"\n";
        out += "  Specular: \"" + m_environment.specular + "\"\n";
    }
    out += "Entities:\n";
    const auto name_of = [](Entity& e) { return e.get_name(); };

    for (Entity* e : children_of(nullptr)) {
        write_subtree(out, *e, "", name_of);
    }
    return out;
}

bool Scene::save(const std::string& path){

    if (!path.empty()) {
        m_path = path;
        Engine::get_instance()->update_window_title();
    }

    std::ofstream file(m_path, std::ios::binary);
    if (!file) {
        printf("*** Error saving scene %s ***\n", m_path.c_str());
        return false;
    }

    // Los materiales que vienen del archivo del modelo tambien se guardan. Con modelos cargando
    // aun se ve el cubo provisional, y sus materiales no son los que tocan.
    if (!Engine::get_instance()->m_resource.is_loading_models()) {
        for (auto [entity, mesh] : m_ecs.viewComponents<MeshComponent>().each()) {
            if (!mesh.has_own_materials()) mesh.store_materials_from_model();
        }
    }

    if (CameraComponent* camera = Engine::get_instance()->get_camera()) {
        file << "Camera:\n";
        file << "  Position: " << format_vec3(camera->get_position()) << "\n";
        file << "  Pitch: " << format_float(camera->get_pitch()) << "\n";
        file << "  Yaw: " << format_float(camera->get_yaw()) << "\n";
    }
    file << serialize();
    printf("Scene saved: %s\n", m_path.c_str());
    return file.good();
}

// ---------------------------------------------------------------------------
// Edicion de entidades
// ---------------------------------------------------------------------------

std::string Scene::unique_name(const std::string& base){

    const auto exists = [&](const std::string& name) {
        for (Entity& e : m_ecs.get_entities()) {
            if (e.get_name() == name) return true;
        }
        return false;
    };

    if (!exists(base)) return base;

    for (int i = 1;; i++) {
        const std::string candidate = base + " (" + std::to_string(i) + ")";
        if (!exists(candidate)) return candidate;
    }
}

void Scene::create_entity_from_template(EntityTemplate type, Vec3 position){

    m_commands.push_back([this, type, position]() {

        const auto mesh = [](const char* model) {
            return "    MeshComponent: \"data/models/basics/" + std::string(model) + "\"\n"
                   "    Material:\n"
                   "      - {Albedo: [0.8, 0.8, 0.8]}\n"
                   "      - {Metallic: 0.0}\n"
                   "      - {Roughness: 0.6}\n";
        };

        std::string base = "Entity";
        std::string body;
        switch (type) {
        case EntityTemplate::Empty: base = "Empty"; break;
        case EntityTemplate::Cube: base = "Cube"; body = mesh("cube.fbx"); break;
        case EntityTemplate::Sphere: base = "Sphere"; body = mesh("sphere_low.obj"); break;
        case EntityTemplate::Cylinder: base = "Cylinder"; body = mesh("cylinder_high.fbx"); break;
        case EntityTemplate::Cone: base = "Cone"; body = mesh("cone_high.fbx"); break;
        case EntityTemplate::PointLight:
            base = "Point Light";
            body = "    Light:\n      Type: \"Point\"\n      Color: [1, 1, 1]\n      Distance: 15\n      Intensity: 5\n"
                   "      Attenuation: 1\n      FallOff: 7.5\n      FallStart: 0.2\n      Enabled: true\n";
            break;
        case EntityTemplate::SpotLight:
            base = "Spot Light";
            body = "    Light:\n      Type: \"Spot\"\n      Direction: [0, -1, 0]\n      Color: [1, 1, 1]\n      Distance: 20\n"
                   "      Intensity: 5\n      InnerAngle: 25\n      OuterAngle: 40\n      Enabled: true\n";
            break;
        case EntityTemplate::DirectionalLight:
            base = "Directional Light";
            body = "    Light:\n      Type: \"Directional\"\n      Direction: [0.5, -1, 0.5]\n      Color: [1, 1, 1]\n      Intensity: 1\n      Enabled: true\n";
            break;
        }

        const std::string name = unique_name(base);
        std::string text = "Entities:\n  " + yaml_string(name) + ":\n    Transform:\n";
        text += "      Position: " + format_vec3(position) + "\n      Rotation: [0, 0, 0]\n      Scale: [1, 1, 1]\n";
        text += body;

        load_text(text);
        select_entity(m_ecs.get_entity_by_name(name));
    });
    request_history_check();
}

void Scene::duplicate_entity(unsigned int id){

    m_commands.push_back([this, id]() {

        for (Entity& original : m_ecs.get_entities()) {
            if (original.get_id() != id) continue;

            // Nombres nuevos para toda la rama; se reservan a medida que se escriben
            std::unordered_map<unsigned int, std::string> names;
            std::vector<std::string> reserved;
            const auto name_of = [&](Entity& e) {
                auto it = names.find(e.get_id());
                if (it != names.end()) return it->second;

                std::string candidate = unique_name(e.get_name());
                int counter = 1;
                while (std::find(reserved.begin(), reserved.end(), candidate) != reserved.end()) {
                    candidate = e.get_name() + " (" + std::to_string(counter++) + ")";
                    candidate = unique_name(candidate);
                }
                reserved.push_back(candidate);
                names[e.get_id()] = candidate;
                return candidate;
            };

            std::string parent_name;
            TransformComponent* transform = m_ecs.get_component<TransformComponent>(original);
            if (transform && transform->get_parent()) {
                for (Entity& e : m_ecs.get_entities()) {
                    if (m_ecs.get_component<TransformComponent>(e) == transform->get_parent()) {
                        parent_name = e.get_name();
                        break;
                    }
                }
            }

            std::string text = "Entities:\n";
            write_subtree(text, original, parent_name, name_of);
            const std::string root_name = names[id];
            load_text(text);

            Entity copy = m_ecs.get_entity_by_name(root_name);
            m_ecs.move_entity(copy.get_id(), id, true);
            select_entity(m_ecs.get_entity_by_name(root_name));
            return;
        }
    });
    request_history_check();
}

void Scene::delete_entity(unsigned int id){

    m_commands.push_back([this, id]() {

        // La entidad y toda su rama
        std::vector<unsigned int> to_delete = { id };
        for (size_t i = 0; i < to_delete.size(); i++) {
            for (Entity& e : m_ecs.get_entities()) {
                if (e.get_id() != to_delete[i]) continue;
                if (TransformComponent* t = m_ecs.get_component<TransformComponent>(e)) {
                    for (Entity* child : children_of(t)) {
                        to_delete.push_back(child->get_id());
                    }
                }
                break;
            }
        }

        for (unsigned int value : to_delete) {
            if (m_selected_entity.get_id() == value) {
                m_selected_entity = Entity(static_cast<unsigned int>(-1), "");
            }
        }

        // De hijos a padres
        for (auto it = to_delete.rbegin(); it != to_delete.rend(); ++it) {
            m_ecs.destroy_entity(*it);
        }
    });
    request_history_check();
}

void Scene::rename_entity(unsigned int id, const std::string& name){

    if (name.empty()) return;

    for (Entity& e : m_ecs.get_entities()) {
        if (e.get_id() == id) {
            if (e.get_name() == name) return;
            const std::string final_name = unique_name(name);
            m_ecs.rename_entity(id, final_name);
            if (m_selected_entity.get_id() == id) m_selected_entity.set_name(final_name);
            request_history_check();
            return;
        }
    }
}

void Scene::sync_light_transforms(){

    const auto world_position = [](TransformComponent& t) {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, t.get_transform());
        return Vec3(world._41, world._42, world._43);
    };

    for (auto [entity, light, transform] : m_ecs.viewComponents<PointLight, TransformComponent>().each()) {
        light.set_position(world_position(transform));
    }

    for (auto [entity, light, transform] : m_ecs.viewComponents<SpotLight, TransformComponent>().each()) {
        light.set_position(world_position(transform));
    }
}

// ---------------------------------------------------------------------------
// Historial
// ---------------------------------------------------------------------------

void Scene::reset_history(){

    m_undo.clear();
    m_redo.clear();
    m_current_state = serialize();
    m_history_check = false;
}

void Scene::record_history(){

    std::string state = serialize();
    if (state == m_current_state) return;

    constexpr size_t max_states = 200;
    if (m_undo.size() >= max_states) m_undo.erase(m_undo.begin());

    m_undo.push_back(std::move(m_current_state));
    m_current_state = std::move(state);
    m_redo.clear();
}

void Scene::apply_snapshot(const std::string& text){

    const std::string selected = m_selected_entity.get_name();
    const bool had_selection = m_selected_entity.get_id() != static_cast<unsigned int>(-1);

    m_ecs.clear();
    m_selected_entity = Entity(static_cast<unsigned int>(-1), "");
    load_text(text);

    if (had_selection) {
        Entity e = m_ecs.get_entity_by_name(selected);
        if (e.get_id() != static_cast<unsigned int>(-1)) m_selected_entity = e;
    }
}

void Scene::undo(){

    m_commands.push_back([this]() {
        if (m_undo.empty()) return;
        m_redo.push_back(m_current_state);
        m_current_state = std::move(m_undo.back());
        m_undo.pop_back();
        apply_snapshot(m_current_state);
    });
}

void Scene::redo(){

    m_commands.push_back([this]() {
        if (m_redo.empty()) return;
        m_undo.push_back(m_current_state);
        m_current_state = std::move(m_redo.back());
        m_redo.pop_back();
        apply_snapshot(m_current_state);
    });
}
