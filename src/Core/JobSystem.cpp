#include "Core/JobSystem.h"


JobSystem::JobSystem() {
	m_stop = false;
	m_ready = false;

	auto threads_count = std::thread::hardware_concurrency() - 1;
	printf("Total threads-> %d\n", threads_count);

	// Create as many threads as can
	for (unsigned int i = 0; i < threads_count; i++) {
		auto thread_worker = [this]() {
			do {
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> locked{ m_mutex_tasks };
					// wait until a task appears
					m_condition_variable.wait(locked, [this]() {
						return !m_tasks_queue.empty() || m_stop;
						});

					if (!m_tasks_queue.empty()) {
						task = m_tasks_queue.front();
						m_tasks_queue.pop();
					}
				}

				if (task) task();
			} while (!m_stop);
			};

		printf("Created thread %d...\n", i);
		m_thread_vector.push_back(std::thread{ std::move(thread_worker) });
	}
}

JobSystem::~JobSystem() {
	m_stop = true;
	m_condition_variable.notify_all();
	int count = 1;

	// Wait until all threads finish before destroy
	for (auto& t : m_thread_vector) {
		if (t.joinable()) {
			t.join();
			printf("Thread %d closed...\n", count);
		}
		count++;
	}
}

void JobSystem::wait_until_finish() {

	/*
	std::unique_lock<std::mutex> locked{ m_mutex_tasks };
	m_condition_variable.wait(locked, [this]() {
		return m_tasks_queue.empty() && m_active_tasks == 0;
	});
	*/

	while (m_tasks_queue.empty() && m_active_tasks > 0);
}


