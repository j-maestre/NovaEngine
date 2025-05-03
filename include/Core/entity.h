#pragma once
#include <entt/single_include/entt/entt.hpp>

class Entity {

public:
	Entity(unsigned int id);
	Entity(const Entity&);
	Entity(Entity&&);
	~Entity();

	unsigned int get_id();
private:

	unsigned int m_id;
};