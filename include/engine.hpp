// engine.hpp

#pragma once

#include "defs.hpp"       // types

#include "attacks.hpp"    // generate attack tables

#include "searchinfo.hpp" // searchinfo
#include "movelist.hpp"   // pvline

#include "tt.hpp"         // transpositiontable
#include "threads.hpp"    // enginethreadpool


namespace MPChess {

namespace Engine {

struct Options {
    std::size_t num_pvs     = 1;
    std::size_t num_threads = 1;
};


inline Options    options;
inline SearchInfo search_info;

inline TranspositionTable tt(Constants::DEFAULT_TABLE_SIZE_MB);
inline EngineThreadPool   thread_pool(options.num_threads);

inline Board engine_board;
inline std::vector<PVLine> pv_lines(options.num_pvs);

inline Types::Milliseconds uci_update_frequency{3000};
inline Types::TimePoint    prev_uci_update_time;

} // Engine namespace

} // MPChess namespace