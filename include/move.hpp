// move.hpp

#pragma once

#include "defs.hpp" // types

#include <iostream> // cout
#include <array>    // array


// A move is a 16 bit integer encoded with
// various information needed to update game state
// 
// Move (16 bits total)
// bits 0-5:   from square is number between 0..63
// bits 6-11:  to square is number between 0..63
// bits 12-15: flags
// bit  12:    promotion flag
// bit  13:    capture flag
// bit  14/15: special (promotion piece , long/short castle)
// 
// special flags (4 bits)
// 1.  quiet move       (0000) = 0
// 2.  double pawn push (0001) = 1
// 3.  short castle     (0010) = 2
// 4.  long castle      (0011) = 3
// 5.  captures         (0100) = 4
// 6.  enpassant        (0101) = 5
// 7.  promote knight   (1000) = 8
// 8.  promote bishop   (1001) = 9
// 9.  promote rook     (1010) = 10
// 10. promote queen    (1011) = 11
// 11. promote knight w/ capture (1100) = 12
// 12. promote bishop w/ capture (1101) = 13
// 13. promote rook   w/ capture (1110) = 14
// 14. promote queen  w/ capture (1111) = 15
//
// Other information (e.g. color moved, piece captured, check, etc.)
// will be inferred from board state

namespace MPChess {

namespace Constants {

namespace Move {

namespace Flags {
    inline constexpr Types::MoveFlag QUIET                  = 0b0000;
    inline constexpr Types::MoveFlag DOUBLE_PAWN_PUSH       = 0b0001;
    inline constexpr Types::MoveFlag SHORT_CASTLE           = 0b0010;
    inline constexpr Types::MoveFlag LONG_CASTLE            = 0b0011;
    inline constexpr Types::MoveFlag CAPTURE                = 0b0100;
    inline constexpr Types::MoveFlag ENPASSANT              = 0b0101;

    inline constexpr Types::MoveFlag PROMOTE_KNIGHT_QUIET   = 0b1000;
    inline constexpr Types::MoveFlag PROMOTE_BISHOP_QUIET   = 0b1001;
    inline constexpr Types::MoveFlag PROMOTE_ROOK_QUIET     = 0b1010;
    inline constexpr Types::MoveFlag PROMOTE_QUEEN_QUIET    = 0b1011;

    inline constexpr Types::MoveFlag PROMOTE_KNIGHT_CAPTURE = 0b1100;
    inline constexpr Types::MoveFlag PROMOTE_BISHOP_CAPTURE = 0b1101;
    inline constexpr Types::MoveFlag PROMOTE_ROOK_CAPTURE   = 0b1110;
    inline constexpr Types::MoveFlag PROMOTE_QUEEN_CAPTURE  = 0b1111;
}

namespace Masks {
    inline constexpr Types::MoveMask FROM_SQ = 0b111111 <<  0; // 0000 0000 0011 1111
    inline constexpr Types::MoveMask TO_SQ   = 0b111111 <<  6; // 0000 1111 1100 0000
    inline constexpr Types::MoveMask FLAG    = 0b1111   << 12; // 1111 0000 0000 0000
    inline constexpr Types::MoveMask CAPTURE = 1 << 14;        // 0100 0000 0000 0000
    inline constexpr Types::MoveMask PROMOTE = 1 << 15;        // 1000 0000 0000 0000
}

} // Move namespace

} // Constants namespace

class Move {
private:

    Types::MoveData move_data;

public:

    // constructors

    Move() = default; // null move
    Move(Types::MoveData move);
    Move(Types::Square from, Types::Square to, Types::MoveFlag flags);

    // getters

    Types::MoveFlag  get_flag() const;
    Types::MoveData  get_data() const;

    bool             is_capture()          const;
    bool             is_promote()          const;
    bool             is_castle()           const;
    bool             is_double_pawn_push() const;
    bool             is_enpassant()        const;
    bool             is_null()             const;

    Types::Square    get_from_square()        const;
    Types::Square    get_to_square()          const;
    Types::PieceType get_promote_piece_type() const;
    Types::Castle    get_castle()             const;

}; // Move class


class OrderedMove : private Move {
private:
    
    Types::MoveScore score;

public:

    // constructors

    OrderedMove() = default; // null move
    using Move::Move;        // inherit constructors


    // get/set score

    Types::MoveScore get_score() const;
    void             set_score(Types::MoveScore score);

}; // OrderedMove class

// non-member operators
bool operator<  (OrderedMove m1, OrderedMove m2);
bool operator>  (OrderedMove m1, OrderedMove m2);
bool operator== (Move m1, Move m2);


// non-member utils
void print_move(Move move, std::ostream& os = std::cout);

} // MPChess namespace