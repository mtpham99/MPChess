// threads.cpp

#include "threads.hpp"

#include "defs.hpp"       // types, constants
#include "utils.hpp"      // current_time

#include "search.hpp"     // search

#include "engine.hpp"     // engine globals
#include "uci.hpp"        // uci (std::cout debug)

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

// EngineThread

EngineThread::EngineThread(std::size_t id) :
    id{id},
    status{Types::EngineThreadStatus::IDLE},
    thread(&EngineThread::loop, this)
{

}

EngineThread::~EngineThread() {

    std::unique_lock<std::mutex> lock(this->mutex);
    this->status = EngineThreadStatus::EXITING;
    lock.unlock();
    
    this->cv.notify_all();
    this->thread.join();
}


// thread loop

void EngineThread::loop() {

    while (true) {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->status = EngineThreadStatus::IDLE;
        this->cv.notify_all(); // notify for any wait_until_stop()

        this->cv.wait(lock, [this]{
            return this->status == EngineThreadStatus::RUNNING
                || this->status == EngineThreadStatus::EXITING;
        });

        // exit/shutdown thread
        if (this->status == EngineThreadStatus::EXITING) {
            break;
        }

        lock.unlock();
        search(*this);

        // reset counter after search
        this->node_counter = 0;
    }
}

void EngineThread::start_search() {

    std::unique_lock<std::mutex> lock(this->mutex);
    this->status = EngineThreadStatus::RUNNING;
    lock.unlock();
    this->cv.notify_all();
}

void EngineThread::stop_search() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->status = EngineThreadStatus::IDLE;
    lock.unlock();
    this->cv.notify_all();
}

void EngineThread::wait_until_stopped() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cv.wait(lock, [this]{
        return this->status == EngineThreadStatus::IDLE;
    });
}

bool EngineThread::check_stop() const {

    std::size_t total_node_counter       = Engine::thread_pool.sum_threads(&EngineThread::node_counter); // this is total since engine started current search
    Engine::search_info.depth_node_count = total_node_counter;                                           // this gets reset to 0 at the start of each iterative deepening iteration

    if (!Engine::search_info.infinite) {
        
        const Milliseconds time_spent = current_time() - Engine::search_info.start_time;
        
        const bool hit_max_nodes = total_node_counter >= Engine::search_info.max_nodes;
        const bool hit_max_time  = time_spent         >= Engine::search_info.max_time;

        if (hit_max_nodes || hit_max_time) {
            Engine::thread_pool.stop_search();
            return true;
        }
    }

    return false;
}


// utils

bool EngineThread::is_main_thread() const {
    return this->id == 0;
}



// EngineThreadPool

EngineThreadPool::EngineThreadPool(std::size_t num_threads) :
    num_threads{num_threads},
    status{EnginePoolStatus::IDLE}
{
    for (std::size_t thread_id = 0; thread_id < this->num_threads; ++thread_id) {
        this->thread_pool.emplace_back(std::make_unique<EngineThread>(thread_id));
    }
}


// statu

bool EngineThreadPool::is_running() const {
    return this->status == EnginePoolStatus::RUNNING;
}

void EngineThreadPool::start_search(SearchInfo&& search_info) {

    // stop any current search
    if (this->is_running()) {
        this->stop_search();
    }

    Engine::search_info = std::move(search_info);

    for (auto pp_thread   = this->thread_pool.rbegin();
              pp_thread  != this->thread_pool.rend();
            ++pp_thread) 
    {
        (*pp_thread)->start_search();
    }

    this->status = EnginePoolStatus::RUNNING;
}

void EngineThreadPool::stop_search() {
    
    for (auto pp_thread   = this->thread_pool.rbegin();
              pp_thread  != this->thread_pool.rend();
            ++pp_thread)
    {
        (*pp_thread)->stop_search();
        (*pp_thread)->wait_until_stopped();
    }

    this->status = EnginePoolStatus::IDLE;
}


// utils

uint64_t EngineThreadPool::sum_threads(std::atomic<uint64_t> EngineThread::* member) const {

    uint64_t sum = 0;
    for (auto& p_thread : this->thread_pool) {
        sum += (*p_thread.*member).load(std::memory_order_relaxed);
    }

    return sum;
}

} // MPChess namespace