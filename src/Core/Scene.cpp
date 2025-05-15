#pragma once
#include "Core/Scene.h"
#include "Core/defines.h"
#include "components/transform_component.h"
#include "components/camera_component.h"

Scene::Scene() : m_ecs(){

}

Scene::Scene(const Scene&)
{
}

Scene::Scene(Scene&&)
{
}

Scene::~Scene()
{
}

void Scene::update(){
	
	auto transforms = m_ecs.viewComponents<TransformComponent>();
	
	for (auto [entity, trans] : transforms.each()) {
		trans.update();
	}


}
