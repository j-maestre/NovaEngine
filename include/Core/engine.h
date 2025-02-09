#pragma once


class Engine final {

private:
	Engine();

public:
	Engine(Engine&&) = delete;
	Engine(const Engine&);
	~Engine();

	static Engine* get_instance() {
		static Engine e;
		return &e;
	}

	void update();
private:
};