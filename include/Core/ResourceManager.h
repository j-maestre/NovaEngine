#pragma once

#include "Core/defines.h"
#include "render/texture.h"
#include <unordered_map>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "render/material.h"
#include "Core/mesh.h"

#include "Core/JobSystem.h"
#include <mutex>

class Engine;
class MeshComponent;

struct TextureToLoad {
	Texture* tex;
	ImageData* data;
	unsigned char* pixels;
};

struct ModelToLoad {
	std::shared_ptr<Model> model;
	MeshComponent* mesh_component;
};

class ResourceManager {

private:
	ResourceManager();
	void set_engine(Engine* e);
	friend class Engine;

public:
	ResourceManager(const ResourceManager&);
	ResourceManager(ResourceManager&&);
	~ResourceManager();

	void load_cube_map(std::string path = "");

	Texture* load_texture(std::string path);
	Texture* load_texture_with_flip(std::string path, unsigned int flip = 1);
	Texture* get_texture(unsigned int id);

	Model* load_mesh(std::string path);
	void load_mesh_async(std::string path, MeshComponent* mesh_comp);

	__forceinline ID3D11ShaderResourceView* get_skybox_srv() { return m_skybox_srv; }
	__forceinline ID3D11Resource** get_skybox_texture() { return m_skybox_texture; }
	unsigned int get_skybox_max_miplevel() { return m_cubemap_max_mip_level; }
	__forceinline ID3D11ShaderResourceView* get_brdf_srv() { return m_brdf_srv; }
	__forceinline ID3D11Resource** get_brdf_texture() { return m_brdf_texture; }


private:
	friend class Engine;
	
	Model* load_mesh(std::string path, bool async);

	Texture* load_texture(std::string path, bool async);
	void ProcessNode(Model* mesh, aiNode* node, const aiScene* scene, std::string absolute_path, bool async = false);
	void ProcessMesh(Mesh* mesh, aiMesh* assimp_mesh, const aiScene* scene, std::string absolute_path, bool async = false);

	void check_models_to_load();
	void check_textures_to_load();

	std::vector<ModelToLoad> m_mesh_component_to_load;
	std::vector<std::shared_ptr<Model>> m_model_to_load;
	std::vector<std::shared_ptr<TextureToLoad>> m_texture_to_load;

	JobSystem m_job_system;
	ID3D11ShaderResourceView* m_skybox_srv = nullptr;
	ID3D11Resource** m_skybox_texture = nullptr;
	unsigned int m_cubemap_max_mip_level;

	ID3D11ShaderResourceView* m_brdf_srv = nullptr;
	ID3D11Resource** m_brdf_texture = nullptr;

	std::mutex m_mutex_textures;
	std::mutex m_mutex_textures_to_load;
	std::mutex m_mutex_models;
	std::mutex m_mutex_model_to_load;
	std::mutex m_mutex_mesh_component;
	std::mutex m_mutex_mesh_component_to_load;

	std::unordered_map<unsigned int, Texture> m_textures;
	std::unordered_map<unsigned int, Model> m_models;

	Engine* m_engine;
};