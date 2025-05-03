#pragma once
#include "Core/ECS.h"

EntityComponentSystem::EntityComponentSystem() : m_registry(std::make_unique<entt::registry>()){

}



EntityComponentSystem::EntityComponentSystem(EntityComponentSystem&& other) noexcept : m_registry(std::move(other.m_registry)) {
	
}

EntityComponentSystem& EntityComponentSystem::operator=(EntityComponentSystem&& other) noexcept{

	if (this != &other) {
		m_registry = std::move(other.m_registry);
	}

	return *this;
}

EntityComponentSystem::~EntityComponentSystem()
{
}

Entity EntityComponentSystem::create_entity()
{
	auto entity = m_registry->create();
	Entity e{static_cast<unsigned int>(entity)};

	return e;
}
