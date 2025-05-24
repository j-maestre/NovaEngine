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

	EntityComponentSystem m_ecs;

private:

	bool load_scene(std::string path);


	std::string m_full_path;

};