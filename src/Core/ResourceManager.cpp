#include "Core/ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Core/engine.h"

ResourceManager::ResourceManager(){
	
}

void ResourceManager::set_engine(Engine* e){
	m_engine = e;
}

ResourceManager::ResourceManager(const ResourceManager&)
{
}

ResourceManager::ResourceManager(ResourceManager&&)
{
}

ResourceManager::~ResourceManager(){

	for (auto& t : m_textures) {
		stbi_image_free(t.second.get_data()->pixels);
	}
}

Texture ResourceManager::load_texture(std::string path){

	unsigned int hash = std::hash<std::string>{}(path);
	
	if (m_textures.contains(hash)) {
		return Texture(m_textures.find(hash)->first);
	}

	Texture t(hash);
	ImageData* data = t.get_data();

	stbi_set_flip_vertically_on_load_thread(1);
	//stbi_set_flip_vertically_on_load(true);
	data->pixels = stbi_load(path.c_str(), &data->width, &data->height, &data->channels,4);

	if (!data->pixels) {
		printf("*** Error loading image %s ***\n", path.c_str());
		return Texture();
	}

	
	D3D11_TEXTURE2D_DESC texture_desc{};
	texture_desc.Width = data->width;
	texture_desc.Height = data->height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags= D3D11_BIND_SHADER_RESOURCE;
	
	D3D11_SUBRESOURCE_DATA init_data{};
	init_data.pSysMem = data->pixels;
	init_data.SysMemPitch = data->width * 4;
	
	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = m_engine->get_engine_props()->deviceInterface->CreateTexture2D(&texture_desc, &init_data, &texture);
	if (FAILED(hr)) return Texture();
	
	data->texture_view = nullptr;
	hr = m_engine->get_engine_props()->deviceInterface->CreateShaderResourceView(texture, nullptr, &data->texture_view);
	texture->Release();
	if (FAILED(hr)) return Texture();

	m_textures.insert(std::pair(hash, t));
	return t;
}

Texture ResourceManager::get_texture(unsigned int id){

	return m_textures.find(id)->second;
}
