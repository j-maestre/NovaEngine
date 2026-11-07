#pragma once
#include <cmath>
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
	material_slot = other.material_slot;
	bounds_center = other.bounds_center;
	bounds_radius = other.bounds_radius;
}

Mesh::~Mesh()
{
}

void Mesh::compute_bounds(){

	if (vertices.empty()) return;

	float min_p[3] = { vertices[0].pos_x, vertices[0].pos_y, vertices[0].pos_z };
	float max_p[3] = { min_p[0], min_p[1], min_p[2] };
	for (const Vertex& v : vertices) {
		const float p[3] = { v.pos_x, v.pos_y, v.pos_z };
		for (int i = 0; i < 3; i++) {
			if (p[i] < min_p[i]) min_p[i] = p[i];
			if (p[i] > max_p[i]) max_p[i] = p[i];
		}
	}

	bounds_center = { (min_p[0] + max_p[0]) * 0.5f, (min_p[1] + max_p[1]) * 0.5f, (min_p[2] + max_p[2]) * 0.5f };

	float radius_squared = 0.0f;
	for (const Vertex& v : vertices) {
		const float dx = v.pos_x - bounds_center.x;
		const float dy = v.pos_y - bounds_center.y;
		const float dz = v.pos_z - bounds_center.z;
		const float distance_squared = dx * dx + dy * dy + dz * dz;
		if (distance_squared > radius_squared) radius_squared = distance_squared;
	}
	bounds_radius = sqrtf(radius_squared);
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
		material_slot = other.material_slot;
		bounds_center = other.bounds_center;
		bounds_radius = other.bounds_radius;
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
		material_slot = other.material_slot;
		bounds_center = other.bounds_center;
		bounds_radius = other.bounds_radius;

		other.vertices.clear();
		other.indices.clear();
		other.num_indices = 0;
		other.num_vertices = 0;
		other.buffer = nullptr;
		other.index_buffer = nullptr;
	}
	return *this;
}
