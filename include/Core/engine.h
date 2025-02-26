#pragma once
#include "Core/defines.h"
#include <render/window.h>
#include <memory>


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

private:

	std::shared_ptr<EngineProps> m_props;
	

};