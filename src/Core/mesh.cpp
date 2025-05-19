#pragma once
#include "Core/mesh.h"
#include "Core/defines.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const Mesh& other){
	vertices.clear();
	indices.clear();

	vertices = other.vertices;
	indices = other.indices;
	num_indices = other.num_indices;
	num_vertices = other.num_vertices;
	buffer = other.buffer;
	index_buffer = other.index_buffer;
	material = other.material;
}

Mesh::~Mesh()
{
}

Mesh& Mesh::operator=(const Mesh& other){

	if (this != &other) {
		vertices = other.vertices;
		indices = other.indices;
		num_indices = other.num_indices;
		num_vertices = other.num_vertices;
		buffer = other.buffer;
		index_buffer = other.index_buffer;
		material = other.material;
	}
	return *this;
	
}

Mesh& Mesh::operator=(Mesh&& other){

	if (this != &other) {
		vertices.clear();
		indices.clear();

		vertices = other.vertices;
		indices = other.indices;
		num_indices = other.num_indices;
		num_vertices = other.num_vertices;
		buffer = other.buffer;
		index_buffer = other.index_buffer;
		material = other.material;

		other.vertices.clear();
		other.indices.clear();
		other.num_indices = 0;
		other.num_vertices = 0;
		other.buffer = nullptr;
		other.index_buffer = nullptr;
	}
	return *this;
}
