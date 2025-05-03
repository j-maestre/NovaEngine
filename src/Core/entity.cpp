#include "Core/entity.h"

Entity::Entity(unsigned int id){
	m_id = id;
}

Entity::Entity(const Entity& other){
	m_id = other.m_id;
}

Entity::Entity(Entity&& other){
	m_id = other.m_id;
	other.m_id = 0;
}

Entity::~Entity(){

}

unsigned int Entity::get_id(){

	return m_id;
}
