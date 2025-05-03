#include "render/material.h"

MaterialComponent::MaterialComponent(){

	Engine* e = Engine::get_instance();

	set_texture_albedo(e->get_default_albedo_texture());
}

MaterialComponent::MaterialComponent(const MaterialComponent&)
{
}

MaterialComponent::MaterialComponent(MaterialComponent&&)
{
}

MaterialComponent::~MaterialComponent()
{
}
