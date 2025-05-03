#pragma once
#include "Core/entity.h"
#include "Core/ECS.h"

class Scene {

public:
	Scene();
	Scene(const Scene&);
	Scene(Scene&&);
	~Scene();

	void update();

	EntityComponentSystem m_ecs;

private:

};