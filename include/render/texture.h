#pragma once
#include "Core/defines.h"

struct ImageData {
	unsigned char* pixels;
	int width;
	int height;
	int channels;
	ID3D11ShaderResourceView* texture_view;
};

class ResourceManager;
class Renderer;

class Texture {


public:
	Texture();
	Texture(unsigned int id);
	Texture(const Texture&);
	Texture(Texture&&);
	~Texture();

	unsigned int get_id() { return m_id; }

private:
	friend class ResourceManager;
	friend class Renderer;

	ImageData* get_data();
	
	unsigned int m_id;
	ImageData m_data;
};