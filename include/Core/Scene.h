#pragma once
#include "Core/entity.h"
#include "Core/ECS.h"


class Scene {

public:
	Scene(const std::string& path_scene);
	Scene(const Scene&);
	Scene(Scene&&);
	~Scene();

	void update();
	Entity get_selected_entity();
	void select_entity(Entity e);

	EntityComponentSystem m_ecs;

private:

	bool load_scene(std::string path);
	
	Entity m_selected_entity;
	std::string m_full_path;

};