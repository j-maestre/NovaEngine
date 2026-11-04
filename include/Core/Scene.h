#pragma once
#include "Core/entity.h"
#include "Core/defines.h"
#include "Core/ECS.h"
#include <functional>
#include <string>
#include <vector>

class TransformComponent;

enum class EntityTemplate {
	Empty,
	Cube,
	Sphere,
	Cylinder,
	Cone,
	PointLight,
	SpotLight,
	DirectionalLight,
};


class Scene {

public:
	Scene(const std::string& path_scene);
	Scene(const Scene&);
	Scene(Scene&&);
	~Scene();

	void update();
	Entity get_selected_entity();
	void select_entity(Entity e);

	// Guardar. Sin ruta, usa la ultima con la que se cargo o guardo.
	bool save(const std::string& path = "");
	const std::string& get_path() const { return m_path; }

	// La escena completa como YAML, en el mismo formato que se carga
	std::string serialize();

	// Cambios de estructura: se aplican al empezar el frame siguiente, nunca a mitad de un pase
	void create_entity_from_template(EntityTemplate type, Vec3 position);
	void duplicate_entity(unsigned int id);
	void delete_entity(unsigned int id);
	void rename_entity(unsigned int id, const std::string& name);

	// Historial: cada estado es la escena serializada
	void undo();
	void redo();
	bool can_undo() const { return !m_undo.empty(); }
	bool can_redo() const { return !m_redo.empty(); }
	void request_history_check() { m_history_check = true; }
	void reset_history();

	EntityComponentSystem m_ecs;

private:

	bool load_scene(std::string path);
	void load_text(const std::string& text);
	void apply_snapshot(const std::string& text);
	void record_history();
	void sync_light_transforms();

	void write_entity(std::string& out, Entity& e, const std::string& name, const std::string& parent);
	void write_subtree(std::string& out, Entity& e, const std::string& parent, const std::function<std::string(Entity&)>& name_of);
	std::vector<Entity*> children_of(TransformComponent* parent);
	std::string unique_name(const std::string& base);

	Entity m_selected_entity;
	std::string m_full_path;
	std::string m_path;

	std::vector<std::function<void()>> m_commands;
	std::vector<std::string> m_undo;
	std::vector<std::string> m_redo;
	std::string m_current_state;
	bool m_history_check = false;

};