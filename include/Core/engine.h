#pragma once
#include "Core/defines.h"
#include <render/window.h>
#include <memory>
#include <vector>
#include <chrono>
#include "Core/ResourceManager.h"
#include "components/Lights/Light.h"
#include "components/Lights/DirectionalLight.h"
#include "components/Lights/SpotLight.h"
#include "components/Lights/PointLight.h"
#include "render/imgui/imgui_manager.h"

struct EngineProps;


class Engine final {

private:
	Engine();

public:

	static Engine* get_instance() {
		static Engine engine;
		return &engine;
	}
	Engine(Engine&&) = delete;
	Engine(const Engine&) = delete;
	~Engine();

	void init(Window* window);
	void update();
	void release();

	void open_console();


	float get_delta_time();
	float get_fps();
	float get_time();

	Material* get_default_material() {return &m_default_material;}

	void load_default_textures();
	Texture* get_default_albedo_texture();

	Model* get_cube() { return m_cube_model; }
	Model* get_cone() { return m_cone_model; }
	Model* get_cone_high() { return m_cone_high_model; }
	Model* get_cylinder() { return m_cylinder_model; }
	Model* get_cylinder_high() { return m_cylinder_high_model; }
	Model* get_sphere() { return m_sphere_model; }
	Model* get_sphere_medium() { return m_sphere_medium_model; }
	Model* get_sphere_high() { return m_sphere_high_model; }
	//const std::vector<Vertex>& get_cube();
	//const std::vector<Vertex>& get_sphere();

	const Input* get_input() { return &m_input; }

	// Geometry basics
	void init_geometries();

	ResourceManager m_resource;

private:

	Input m_input;

	friend class ResourceManager;
	friend class Renderer;
	friend class Light;
	friend class DirectionalLight;
	friend class SpotLight;
	friend class PointLight;
	friend class ImguiManager;
	inline std::shared_ptr<EngineProps> get_engine_props() { return m_props; }




	std::shared_ptr<EngineProps> m_props;
	D3D11_VIEWPORT m_viewport;
	D3D11_RASTERIZER_DESC m_raster;
	ID3D11RasterizerState* m_raster_state;

	std::vector<Vertex> m_geometry_cube;
	std::vector<Vertex> m_geometry_sphere;


	Model* m_cube_model;
	Model* m_cone_model;
	Model* m_cone_high_model;
	Model* m_cylinder_model;
	Model* m_cylinder_high_model;
	Model* m_sphere_model;
	Model* m_sphere_medium_model;
	Model* m_sphere_high_model;

	Material m_default_material;

	std::chrono::high_resolution_clock::time_point m_last_time;
	float m_delta_time;
	float m_time;

	unsigned int m_default_texture_albedo;
	unsigned int m_texture_tmp;


};