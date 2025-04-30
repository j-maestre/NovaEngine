#include "render/texture.h"

Texture::Texture()
{
}

Texture::Texture(unsigned int id){
	m_id = id;
}

Texture::Texture(const Texture& other){
	m_id = other.m_id;
	m_data = other.m_data;
}

Texture::Texture(Texture&& other){

	m_id = other.m_id;
	m_data = other.m_data;
	other.m_id = 0;
	other.m_data.texture_view = nullptr;
}

Texture::~Texture(){

}

ImageData* Texture::get_data(){

	return &m_data;
}
