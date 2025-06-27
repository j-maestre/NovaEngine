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

private:

};


struct Model {
	std::vector<Mesh> meshes;
	std::vector<Mesh> meshes_copy;
	
};