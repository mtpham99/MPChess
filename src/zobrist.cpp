// zobrist.cpp

#include "zobrist.hpp" 

#include "defs.hpp"    // types, constants
#include "utils.hpp"   // file_index

#include "board.hpp"   // board

#include "rng.hpp"     // xorshift rng

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

namespace Zobrist {

// key getters
Key get_piece_square_key(Piece p, Square sq) {
    const auto index = p + sq*NUM_PIECES;
    return Hashes::piece_square[index];
}

Key get_enpassant_key(Square sq) {
    const auto index = file_index(sq);
    return Hashes::enpassant[index];
}

Key get_castle_key(Castle c) {
    return Hashes::castle[c];
}

Key get_color_key() {
    return Hashes::color;
}

} // Zobrist namespace

} // MPChess namespace