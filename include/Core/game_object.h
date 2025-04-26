#pragma once
#include "Core/defines.h"
#include <render/window.h>
#include <memory>
#include <vector>


class GameObject {

public:
	GameObject();
	GameObject(const GameObject&);
	GameObject(GameObject&&);
	~GameObject();

	//void add_component();
	//void remove_component();

private:
	
	int m_id;
};