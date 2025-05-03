#pragma once

#include "Core/defines.h"
#include "render/texture.h"
#include <unordered_map>

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

private:

	std::unordered_map<unsigned int, Texture> m_textures;
	Engine* m_engine;
};