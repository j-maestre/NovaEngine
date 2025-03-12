#pragma once
#include "memory"
#include <Core/defines.h>
#include <Core/engine.h>

class Renderer {



public:
	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&);
	~Renderer();

	bool init_pipeline(Engine* engine);
	void active_shader(ShaderType type);

	void upload_triangle();
	void draw_triangle();


	void release();
private:

	void compile_shader(std::string path);
	ShaderFiles m_shader_files;
	Engine* m_engine_ptr;

	ID3D11Buffer* m_pVBuffer;
	ID3D11InputLayout* m_pLayout;

	bool m_isInitialized;
	
};
