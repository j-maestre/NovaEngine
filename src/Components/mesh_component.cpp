
#include "components/mesh_component.h"

MeshComponent::MeshComponent(){
	m_model = Engine::get_instance()->get_cube();
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
