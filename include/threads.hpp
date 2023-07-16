// threads.hpp

#pragma once

#include "defs.hpp"           // types
#include "board.hpp"          // board
#include "movelist.hpp"       // movelist

#include <thread>             // thread
#include <atomic>             // atomics
#include <mutex>              // mutex
#include <condition_variable> // cv

#include <vector>             // vector
#include <memory>             // unique pointer


namespace MPChess {

// Forward declaration

struct SearchInfo;


namespace Types {

enum class EngineThreadStatus : int {
    IDLE,
    RUNNING,
    EXITING
};

enum class EnginePoolStatus : int {
    IDLE,
    RUNNING,
};

} // Types namespace

class EngineThread {
private:

    std::size_t id;
    std::atomic<Types::EngineThreadStatus> status;
    std::thread thread;

    std::mutex              mutex;
    std::condition_variable cv;

    Board           root_board;
    RegularMoveList root_moves;
    
    std::atomic<uint64_t> node_counter;

public:

    // constructors/destructors

    EngineThread(std::size_t id);
    ~EngineThread();


    // no copying

    EngineThread(const EngineThread& engine_thread)             = delete;
    EngineThread& operator= (const EngineThread& engine_thread) = delete;


    // thread loop

    void loop();
    void start_search();
    void stop_search();
    void wait_until_stopped();
    bool check_stop() const;


    // search

    friend Types::Eval search(EngineThread& thread);
    friend Types::Eval alpha_beta(EngineThread& thread, Types::Depth depth, Types::Eval alpha, Types::Eval beta, bool root, RegularMoveList& pv_parent);
    friend Types::Eval quiescence(EngineThread& thread, Types::Eval alpha, Types::Eval beta);


    // utils

    bool is_main_thread() const;
};


class EngineThreadPool {
private:

    std::size_t                                num_threads;
    std::atomic<Types::EnginePoolStatus>       status;
    std::vector<std::unique_ptr<EngineThread>> thread_pool;

public:

    // constructors

    EngineThreadPool(std::size_t num_threads = 1);


    // status

    bool is_running() const;
    void start_search(SearchInfo&& search_info);
    void stop_search();


    // utils

    uint64_t sum_threads(std::atomic<uint64_t> EngineThread::* member) const;

};

} // MPChess namespace