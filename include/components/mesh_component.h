#pragma once
#include "Core/defines.h"
#include "Core/input.h"
#include "Core/engine.h"
#include "memory"



class MeshComponent {

public:
	MeshComponent();
	MeshComponent(Model* model);
	MeshComponent(const MeshComponent&);
	MeshComponent(MeshComponent&&);
	~MeshComponent();

	void set_model(Model* model);
	inline Model* get_model() { return m_model; }

private:
	
	Model* m_model;
	unsigned int m_id;

};