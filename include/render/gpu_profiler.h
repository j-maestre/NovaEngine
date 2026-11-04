#pragma once
#include "Core/defines.h"

// Pases medidos, en el orden en que se ejecutan
enum class GpuPass : int {
	Clear = 0,
	Depth,
	GBuffer,
	SSAO,
	SSAOBlur,
	SSAOMix,
	Lights,
	Skybox,
	Bloom,
	Composite,
	Count,
};

constexpr int GPU_PASS_COUNT = static_cast<int>(GpuPass::Count);

// Tiempos de GPU con timestamp queries de D3D11.
// Los resultados llegan con unos frames de retraso: nunca se espera a la GPU para leerlos.
class GpuProfiler {

public:
	GpuProfiler() = default;
	GpuProfiler(const GpuProfiler&) = delete;
	GpuProfiler& operator=(const GpuProfiler&) = delete;
	~GpuProfiler();

	void init(ID3D11Device* device);

	void begin_frame(ID3D11DeviceContext* context);
	// Marca el final de un pase: su tiempo va desde la marca anterior hasta esta
	void mark(ID3D11DeviceContext* context, GpuPass pass);
	void end_frame(ID3D11DeviceContext* context);

	// Milisegundos por pase, suavizados
	const float* get_pass_ms() const { return m_pass_ms; }

	static const char* get_pass_name(int pass);

private:

	static constexpr int k_frames = 5;

	struct Frame {
		ID3D11Query* disjoint = nullptr;
		ID3D11Query* begin = nullptr;
		ID3D11Query* stamps[GPU_PASS_COUNT] = {};
		bool issued[GPU_PASS_COUNT] = {};
		bool pending = false;
	};

	void collect(ID3D11DeviceContext* context, Frame& frame);

	Frame m_frames[k_frames];
	int m_write_index = 0;
	bool m_initialized = false;
	float m_pass_ms[GPU_PASS_COUNT] = {};
};
