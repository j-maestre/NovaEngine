#pragma once
#include <entt/single_include/entt/entt.hpp>

class Entity {

public:
	Entity(unsigned int id, std::string name);
	Entity(const Entity&);
	Entity(Entity&&);
	~Entity();

	Entity& operator=(const Entity&);

	unsigned int get_id();
	std::string get_name();
	void set_name(const std::string& name) { m_name = name; }
private:

	unsigned int m_id;
	std::string m_name;
};