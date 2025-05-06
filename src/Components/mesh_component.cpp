
#include "components/mesh_component.h"

MeshComponent::MeshComponent(){
	m_model = Engine::get_instance()->get_cube();
	set_material(*(Engine::get_instance()->get_default_material()));
}

MeshComponent::MeshComponent(Model* model){
	m_model = model;
}

MeshComponent::MeshComponent(const MeshComponent&)
{
}

MeshComponent::MeshComponent(MeshComponent&&)
{
}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::set_model(Model* model){
	m_model = model;
}

void MeshComponent::set_material(Material mat){
	for (auto& meshes : m_model->meshes) {
		meshes.material = mat;
	}
}
