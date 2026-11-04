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

Entity EntityComponentSystem::create_entity(std::string name){

	auto entity = m_registry->create();
	Entity e{static_cast<unsigned int>(entity), name};

	m_entities.push_back(e);
	return e;
}

Entity EntityComponentSystem::get_entity_by_name(std::string name){
	for (auto& e : m_entities) {
		if (e.get_name() == name) {
			return e;
		}
	}

	return Entity(-1, "bad entity");
	 
}

void EntityComponentSystem::destroy_entity(unsigned int id){

	m_registry->destroy(static_cast<entt::entity>(id));
	for (size_t i = 0; i < m_entities.size(); i++) {
		if (m_entities[i].get_id() == id) {
			m_entities.erase(m_entities.begin() + i);
			return;
		}
	}
}

void EntityComponentSystem::rename_entity(unsigned int id, const std::string& name){

	for (Entity& e : m_entities) {
		if (e.get_id() == id) {
			e.set_name(name);
			return;
		}
	}
}

void EntityComponentSystem::clear(){

	m_registry = std::make_unique<entt::registry>();
	m_entities.clear();
}

void EntityComponentSystem::move_entity(unsigned int id, unsigned int target_id, bool after){

	if (id == target_id) return;

	auto find = [&](unsigned int value) {
		for (size_t i = 0; i < m_entities.size(); i++) {
			if (m_entities[i].get_id() == value) return i;
		}
		return m_entities.size();
	};

	const size_t from = find(id);
	if (from == m_entities.size() || find(target_id) == m_entities.size()) return;

	Entity moved = m_entities[from];
	m_entities.erase(m_entities.begin() + from);

	const size_t to = find(target_id) + (after ? 1 : 0);
	m_entities.insert(m_entities.begin() + to, moved);
}
