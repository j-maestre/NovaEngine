#include "Core/entity.h"

Entity::Entity(unsigned int id, std::string name){
	m_id = id;
	m_name = name;
}

Entity::Entity(const Entity& other){
	m_id = other.m_id;
	m_name = other.m_name;
}

Entity::Entity(Entity&& other){
	m_id = other.m_id;
	m_name = other.m_name;

	other.m_id = 0;
	other.m_name.clear();
}

Entity::~Entity(){

}

unsigned int Entity::get_id(){

	return m_id;
}

std::string Entity::get_name()
{
	return m_name;
}
