#pragma once
#include <entt/single_include/entt/entt.hpp>

class Entity {

public:
	Entity();
	Entity(const Entity&);
	Entity(Entity&&);
	~Entity();

private:

	int m_id;
};