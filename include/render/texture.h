#pragma once
#include "Core/defines.h"
#include "stb/stb_image.h"



class ResourceManager;
class Renderer;

struct ImageData {
	int width;
	int height;
	int channels;
	ID3D11ShaderResourceView* texture_view;

	ImageData() {
		width = 0;
		height = 0;
		channels = 0;
		texture_view = 0;

	}

	~ImageData() {
	}


	ImageData(const ImageData& other) {
		width = other.width;
		height = other.height;
		channels = other.channels;
		texture_view = other.texture_view;
	}

	ImageData(ImageData&& other) noexcept {
		width = other.width;
		height = other.height;
		channels = other.channels;
		texture_view = other.texture_view;


		other.texture_view = nullptr;
	}

	ImageData& operator=(ImageData&& other) {
		if (this != &other) {

			width = other.width;
			height = other.height;
			channels = other.channels;
			texture_view = other.texture_view;

			other.texture_view = nullptr;
		}

		return *this;
	}

	ImageData& operator=(const ImageData& other) {
		if (this != &other) {

			width = other.width;
			height = other.height;
			channels = other.channels;
			texture_view = other.texture_view;
		}

		return *this;
	}
};


class Texture {


public:
	Texture();
	Texture(unsigned int id);
	Texture(const Texture&);
	Texture(Texture&&);
	~Texture();

	Texture& operator=(const Texture&);
	Texture& operator=(Texture&&);

	unsigned int get_id();

private:
	friend class ResourceManager;
	friend class Renderer;

	ImageData* get_data();
	
	unsigned int m_id;
	ImageData m_data;
};