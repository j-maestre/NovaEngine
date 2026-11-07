#pragma once
#include "Core/defines.h"
#include "memory"
#include "render/material.h"

class Mesh {

public:

	Mesh();
	Mesh(const Mesh&);
	~Mesh();

	Mesh& operator=(const Mesh&);
	Mesh& operator=(Mesh&&);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int num_indices;
	unsigned int num_vertices;
	ID3D11Buffer* buffer = nullptr;
	ID3D11Buffer* index_buffer = nullptr;

	Material material;
	unsigned int material_slot = 0;	// Material del archivo original al que pertenece esta malla

	// Esfera que envuelve la malla, en espacio local. Radio negativo: sin calcular, nunca se descarta.
	Vec3 bounds_center = { 0.0f, 0.0f, 0.0f };
	float bounds_radius = -1.0f;
	void compute_bounds();

private:

};


struct Model {
	std::vector<Mesh> meshes;
	std::vector<Mesh> meshes_copy;

	bool m_engine_default = false;
	
};