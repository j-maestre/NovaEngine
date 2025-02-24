#pragma once
#include "Core/defines.h"
#include <memory>


struct EngineProps;

class Engine final {

private:

public:
	Engine();
	Engine(Engine&&) = delete;
	Engine(const Engine&) = delete;
	~Engine();


	void init(const WindowInfo* window_used);
	void release();

	void update();
private:

	std::shared_ptr<EngineProps> m_props;
	

};