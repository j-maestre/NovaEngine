#pragma once

#include "Core/defines.h"
#include "render/texture.h"
#include <unordered_map>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "render/material.h"

class Engine;



class ResourceManager {


private:
	ResourceManager();
	void set_engine(Engine* e);
	friend class Engine;

public:
	ResourceManager(const ResourceManager&);
	ResourceManager(ResourceManager&&);
	~ResourceManager();

	Texture* load_texture(std::string path);
	Texture* get_texture(unsigned int id);

	Model* load_mesh(std::string path, MaterialComponent* material = nullptr);

private:

	void ProcessNode(Model* mesh, aiNode* node, const aiScene* scene);
	void ProcessMesh(Mesh* mesh, aiMesh* assimp_mesh, const aiScene* scene);

	std::unordered_map<unsigned int, Texture> m_textures;
	std::unordered_map<unsigned int, Model> m_models;
	Engine* m_engine;
};