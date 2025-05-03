#pragma once
#include <entt/single_include/entt/entt.hpp>

class EntityComponentSystem {

public:
	EntityComponentSystem();
	EntityComponentSystem(const EntityComponentSystem&);
	EntityComponentSystem(EntityComponentSystem&&);
	~EntityComponentSystem();



private:

	int m_id;
};