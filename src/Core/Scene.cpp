#pragma once
#include "Core/Scene.h"
#include "Core/defines.h"
#include "components/transform_component.h"
#include "components/camera_component.h"
#include "render/imgui/imgui_manager.h"
#include <chrono>

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

#ifdef ENABLE_IMGUI
	auto start = std::chrono::high_resolution_clock::now();
#endif
	auto transforms = m_ecs.viewComponents<TransformComponent>();
	
	for (auto [entity, trans] : transforms.each()) {
		trans.update();
	}

#ifdef ENABLE_IMGUI
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = end - start;

	ImguiManager::get_instance()->m_update_transform_time = std::chrono::duration<float>(elapsed).count();
#endif
}
