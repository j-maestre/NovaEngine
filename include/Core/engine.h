#pragma once
#include "Core/defines.h"
#include <render/window.h>
#include <memory>
#include <vector>


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

	const std::vector<Vertex>& get_cube();
	const std::vector<Vertex>& get_sphere();

private:
	friend class Renderer;
	inline std::shared_ptr<EngineProps> get_engine_props() { return m_props; }



	std::shared_ptr<EngineProps> m_props;
	D3D11_VIEWPORT m_viewport;
	D3D11_RASTERIZER_DESC m_raster;
	ID3D11RasterizerState* m_raster_state;

	// Geometry basics
	void init_geometries();
	std::vector<Vertex> m_geometry_cube;
	std::vector<Vertex> m_geometry_sphere;
};