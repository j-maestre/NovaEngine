#include "render/gpu_profiler.h"

GpuProfiler::~GpuProfiler() {

	for (Frame& frame : m_frames) {
		if (frame.disjoint) frame.disjoint->Release();
		if (frame.begin) frame.begin->Release();
		for (ID3D11Query* stamp : frame.stamps) {
			if (stamp) stamp->Release();
		}
	}
}

void GpuProfiler::init(ID3D11Device* device) {

	D3D11_QUERY_DESC disjoint_desc = {};
	disjoint_desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

	D3D11_QUERY_DESC stamp_desc = {};
	stamp_desc.Query = D3D11_QUERY_TIMESTAMP;

	bool ok = true;
	for (Frame& frame : m_frames) {
		ok &= SUCCEEDED(device->CreateQuery(&disjoint_desc, &frame.disjoint));
		ok &= SUCCEEDED(device->CreateQuery(&stamp_desc, &frame.begin));
		for (ID3D11Query*& stamp : frame.stamps) {
			ok &= SUCCEEDED(device->CreateQuery(&stamp_desc, &stamp));
		}
	}
	m_initialized = ok;
}

void GpuProfiler::begin_frame(ID3D11DeviceContext* context) {

	if (!m_initialized) return;

	Frame& frame = m_frames[m_write_index];
	for (bool& issued : frame.issued) issued = false;

	context->Begin(frame.disjoint);
	context->End(frame.begin);
}

void GpuProfiler::mark(ID3D11DeviceContext* context, GpuPass pass) {

	if (!m_initialized) return;

	Frame& frame = m_frames[m_write_index];
	const int index = static_cast<int>(pass);
	context->End(frame.stamps[index]);
	frame.issued[index] = true;
}

void GpuProfiler::end_frame(ID3D11DeviceContext* context) {

	if (!m_initialized) return;

	Frame& current = m_frames[m_write_index];
	context->End(current.disjoint);
	current.pending = true;

	// La siguiente en escribirse es tambien la mas antigua: si ya esta lista, se lee
	m_write_index = (m_write_index + 1) % k_frames;
	Frame& oldest = m_frames[m_write_index];
	if (oldest.pending) {
		collect(context, oldest);
	}
}

void GpuProfiler::collect(ID3D11DeviceContext* context, Frame& frame) {

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint = {};
	if (context->GetData(frame.disjoint, &disjoint, sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) return;

	frame.pending = false;

	// Disjoint: la frecuencia ha cambiado durante el frame y los tiempos no valen
	if (disjoint.Disjoint || disjoint.Frequency == 0) return;

	UINT64 previous = 0;
	if (context->GetData(frame.begin, &previous, sizeof(previous), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) return;

	float pass_ms[GPU_PASS_COUNT] = {};
	for (int i = 0; i < GPU_PASS_COUNT; i++) {
		if (!frame.issued[i]) continue;

		UINT64 stamp = 0;
		if (context->GetData(frame.stamps[i], &stamp, sizeof(stamp), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) return;

		pass_ms[i] = static_cast<float>(static_cast<double>(stamp - previous) / static_cast<double>(disjoint.Frequency) * 1000.0);
		previous = stamp;
	}

	const float smoothing = 0.1f;
	for (int i = 0; i < GPU_PASS_COUNT; i++) {
		m_pass_ms[i] += (pass_ms[i] - m_pass_ms[i]) * smoothing;
	}
}

const char* GpuProfiler::get_pass_name(int pass) {

	static const char* names[GPU_PASS_COUNT] = {
		"Clear",
		"Depth prepass",
		"G-Buffer",
		"SSAO",
		"SSAO blur",
		"Lights",
		"Skybox",
		"Bloom",
		"Composite",
	};
	return names[pass];
}
