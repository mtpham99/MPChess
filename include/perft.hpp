// perft.hpp

#pragma once

namespace MPChess {

// forward declarations
class Board;


namespace Types {

struct PerftInfo {
    unsigned long long captures   = 0;
    unsigned long long enpassants = 0;
    unsigned long long promotions = 0;
    unsigned long long castles    = 0;
    unsigned long long checks     = 0;
};

} // Types namespace


unsigned long long perft(unsigned int depth, Board& board, Types::PerftInfo* p_perft_info=nullptr);

} // MPChess namespace