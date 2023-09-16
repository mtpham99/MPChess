// engine.hpp

#pragma once

#include "defs.hpp"       // types

#include "attacks.hpp"    // generate attack tables

#include "searchinfo.hpp" // searchinfo
#include "movelist.hpp"   // pvline

#include "tt.hpp"         // transpositiontable
#include "threads.hpp"    // enginethreadpool

#include <array>


namespace MPChess {

namespace Types {

using HistoryTable = std::array<std::array<Types::MoveScore, Constants::NUM_SQUARES>, Constants::NUM_PIECES>;
using KillerTable  = std::array<std::array<Move, Constants::NUM_KILLER_MOVES>, Constants::MAX_PLY>;

} // Types namespace

namespace Engine {

struct EngineOptions {
    std::size_t num_pvs     = 1;
    std::size_t num_threads = 1;
    
    // debug mode
#ifndef NDEBUG
    bool debug = false;
#else
    bool debug = true;
#endif

};

inline EngineOptions options;
inline SearchInfo    search_info;

inline Types::HistoryTable history_table;
inline Types::KillerTable  killer_table;
inline TranspositionTable  tt(Constants::DEFAULT_TABLE_SIZE_MB);

inline EngineThreadPool    thread_pool(options.num_threads);

inline Board engine_board;
inline std::vector<PVLine> pv_lines(options.num_pvs);

inline Types::Milliseconds uci_update_frequency{2000};
inline Types::TimePoint    prev_uci_update_time;

} // Engine namespace

} // MPChess namespace