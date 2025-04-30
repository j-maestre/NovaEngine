#pragma once
#include "Core/defines.h"
#include <render/window.h>
#include <memory>
#include <vector>
#include <chrono>
#include "Core/ResourceManager.h"

struct EngineProps;

class Engine final {

private:

public:
	Engine();
	Engine(Engine&&) = delete;
	Engine(const Engine&) = delete;
	~Engine();

	void init(Window* window);
	void update();
	void release();

	float get_delta_time();
	float get_fps();

	void load_default_textures();
	Texture get_default_albedo_texture();

	const std::vector<Vertex>& get_cube();
	const std::vector<Vertex>& get_sphere();

	const Input* get_input() { return &m_input; }

	ResourceManager m_resource;

private:

	Input m_input;

	friend class Renderer;
	friend class ResourceManager;
	inline std::shared_ptr<EngineProps> get_engine_props() { return m_props; }



	std::shared_ptr<EngineProps> m_props;
	D3D11_VIEWPORT m_viewport;
	D3D11_RASTERIZER_DESC m_raster;
	ID3D11RasterizerState* m_raster_state;

	// Geometry basics
	void init_geometries();
	std::vector<Vertex> m_geometry_cube;
	std::vector<Vertex> m_geometry_sphere;

	std::chrono::high_resolution_clock::time_point m_last_time;
	float m_delta_time;

	unsigned int m_default_texture_albedo;
};