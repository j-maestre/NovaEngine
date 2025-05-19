#pragma once
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <vector>
#include <functional>
#include <thread>
#include <queue>
#include <deque>

class JobSystem {
public:

    JobSystem();
    JobSystem(const JobSystem&) = delete;
    JobSystem(JobSystem&&) = delete;
    ~JobSystem();

    // T -> return type
    // Args -> reciving parameters
    // callback -> callback to pass the return value

    template <class T, class... Args>
    void add_task(std::function<T(Args...)>& func, std::function<void(T)> callback, Args... args) {


        m_active_tasks++;
        auto task = [func, args..., callback, this]() {
            // Call result callback passing return value as parameter
            T result = func(args...);
            callback(result);
            m_active_tasks--;
            m_condition_variable.notify_all();
            };

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(task);
        }
        m_condition_variable.notify_one();
    }

    template <class... Args>
    void add_task(std::function<void(Args...)>& func, std::function<void()> callback, Args... args) {

        m_active_tasks++;
        auto task = [func, args..., callback,this]() {
            func(args...);
            callback();
            m_active_tasks--;
            m_condition_variable.notify_all();
            };

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(task);
        }
        m_condition_variable.notify_one();
    }

    template <class... Args>
    void add_task(std::function<void(Args...)>& func, Args... args) {

        m_active_tasks++;
        auto task = [func, args...,this]() {
            func(args...);
            m_active_tasks--;
            m_condition_variable.notify_all();
            };


        //auto task = std::bind(func, std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(task);
        }

        m_condition_variable.notify_one();
    }
    
    
    void add_task(std::function<void()>& func) {

        m_active_tasks++;
        auto task = [func,this]() {
            func();
            m_active_tasks--;
            m_condition_variable.notify_all();
            };


        //auto task = std::bind(func, std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(task);
        }

        m_condition_variable.notify_one();
    }

    /*
    template <class... Args>
    void add_task(std::function<void()>& func) {

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(func);
        }

        m_condition_variable.notify_one();
    }
    */

    /*
    template <class... Args>
    void add_task(std::function<void()> func) { 

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            m_tasks_queue.push(func);
        }

        m_condition_variable.notify_one();
    }
    */

    template <class... Args>
    void add_task(std::vector<std::function<void()>>& func) {

        {
            std::lock_guard<std::mutex> locked{ m_mutex_tasks };
            for (auto& f : func) {
                m_tasks_queue.push(f);
            }
        }

        m_condition_variable.notify_all();
    }


    void wait_until_finish();

    void terminate() { m_stop = true; }


private:
    std::vector<std::thread> m_thread_vector;
    std::queue<std::function<void()>> m_tasks_queue;
    std::mutex m_mutex_tasks;
    std::condition_variable m_condition_variable;
    bool m_ready = false;
    bool m_stop = false;
    std::atomic<int> m_active_tasks;
};


