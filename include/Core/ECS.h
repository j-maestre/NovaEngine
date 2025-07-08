#pragma once
#include <entt/single_include/entt/entt.hpp>
#include "Core/entity.h"
#include <memory>

class EntityComponentSystem {

public:
	EntityComponentSystem();
	EntityComponentSystem(const EntityComponentSystem&) = delete;
	EntityComponentSystem& operator=(const EntityComponentSystem&) = delete;
	
	EntityComponentSystem(EntityComponentSystem&&) noexcept;
	EntityComponentSystem& operator=(EntityComponentSystem&& other) noexcept;
	~EntityComponentSystem();



	Entity create_entity(std::string name = "");

	template <class T, class... Args>
	T& add_component(Entity e,Args&&... args){
		return m_registry->emplace<T>(static_cast<entt::entity>(e.get_id()), std::forward<Args>(args)...);
	}

	template <class T>
	T* get_component(Entity e) {
		return m_registry->try_get<T>(static_cast<entt::entity>(e.get_id()));
	}
	
	template <class T>
	void remove_component(Entity e) {
		m_registry->remove<T>(static_cast<entt::entity>(e.get_id()));
	}

	// Returns every entity that has every component
	template<typename... T>
	auto viewComponents() {
		return m_registry->view<T...>();
	}

	std::vector<Entity>& get_entities() {
		return m_entities;
	}

	Entity get_entity_by_name(std::string name);


private:

	std::unique_ptr<entt::registry> m_registry;
	std::vector<Entity> m_entities;
};