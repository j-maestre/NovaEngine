#pragma once
#include <entt/single_include/entt/entt.hpp>

class Entity {

public:
	Entity(unsigned int id, std::string name);
	Entity(const Entity&);
	Entity(Entity&&);
	~Entity();

	unsigned int get_id();
	std::string get_name();
private:

	unsigned int m_id;
	std::string m_name;
};