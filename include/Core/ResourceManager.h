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
#include <atomic>

class Engine;
class MeshComponent;

struct TextureToLoad {
	Texture* tex;
	ImageData* data;
	unsigned char* pixels;
	bool srgb = false;
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

	// Cubemaps de la escena: cielo, irradiancia y reflejos. Si ya estan cargados no hace nada.
	void set_environment(const std::string& skybox, const std::string& irradiance, const std::string& specular);

	void load_cube_map(std::string path = "");
	void load_irradiance_map(std::string path);
	void load_specular_map(std::string path);

	Texture* load_texture(std::string path);
	// Para texturas de color (albedo, emissive): la GPU las convierte a lineal al leerlas y al generar mips
	Texture* load_texture_srgb(std::string path);

	// Ruta con la que se cargo la textura; vacia si es una de las del motor
	std::string get_texture_path(Texture* texture);

	// Mientras haya cargas en curso no se puede destruir ningun MeshComponent
	bool is_loading_models() const { return m_pending_model_loads > 0; }
	Texture* load_texture_with_flip(std::string path, unsigned int flip = 1);
	Texture* get_texture(unsigned int id);

	Model* load_mesh(std::string path);
	void load_mesh_async(std::string path, MeshComponent* mesh_comp);

	__forceinline ID3D11ShaderResourceView* get_skybox_srv() { return m_skybox_srv; }
	__forceinline ID3D11Resource** get_skybox_texture() { return m_skybox_texture; }
	unsigned int get_specular_max_miplevel() { return m_specular_max_mip_level; }
	__forceinline ID3D11ShaderResourceView* get_brdf_srv() { return m_brdf_srv; }
	__forceinline ID3D11ShaderResourceView* get_irradiance_srv() { return m_irradiance_srv; }
	__forceinline ID3D11ShaderResourceView* get_specular_srv() { return m_specular_srv; }
	__forceinline ID3D11Resource** get_brdf_texture() { return m_brdf_texture; }

	void release();
	void release_non_default();

private:
	friend class Engine;
	
	Model* load_mesh(std::string path, bool async);

	Texture* load_texture(std::string path, bool async, bool srgb = false);
	Texture* load_texture_sync(std::string path, bool srgb);
	void register_texture_path(unsigned int hash, const std::string& path);
	void ProcessNode(Model* mesh, aiNode* node, const aiScene* scene, std::string absolute_path, bool async = false);
	void ProcessNodeTree(Model* model, aiNode* node, const aiScene* scene, const std::string& absolute_path, bool async, const aiMatrix4x4& parent_world, const aiMatrix4x4& to_first_mesh);
	void ProcessMesh(Mesh* mesh, aiMesh* assimp_mesh, const aiScene* scene, std::string absolute_path, bool async = false);

	void check_models_to_load();
	void check_textures_to_load();


	std::vector<ModelToLoad> m_mesh_component_to_load;
	std::atomic<int> m_pending_model_loads{ 0 };
	std::unordered_map<unsigned int, std::string> m_texture_paths;
	std::mutex m_mutex_texture_paths;
	std::vector<std::shared_ptr<Model>> m_model_to_load;
	std::vector<std::shared_ptr<TextureToLoad>> m_texture_to_load;

	JobSystem m_job_system;
	ID3D11ShaderResourceView* m_skybox_srv = nullptr;
	ID3D11Resource** m_skybox_texture = nullptr;

	ID3D11ShaderResourceView* m_brdf_srv = nullptr;
	ID3D11ShaderResourceView* m_irradiance_srv = nullptr;
	ID3D11ShaderResourceView* m_specular_srv = nullptr;		// Entorno prefiltrado: cada mip corresponde a una rugosidad
	unsigned int m_specular_max_mip_level = 0;
	std::string m_environment_paths;
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