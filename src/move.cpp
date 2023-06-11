// move.cpp

#include "move.hpp"

#include "defs.hpp"  // types, constants
#include "utils.hpp" // rank_index, file_index

using namespace MPChess::Types;
using namespace MPChess::Constants;
using namespace MPChess::Constants::Move;


namespace MPChess {

// Move constructors

Move::Move(MoveData data) : move_data{data} {

}

Move::Move(Square from, Square to, MoveFlag flags) :
    move_data{static_cast<MoveData>( (from << 0) | (to << 6) | (flags << 12) )}
{

}


// getters

MoveFlag Move::get_flag() const {
    return static_cast<MoveFlag>( (this->move_data & Masks::FLAG) >> 12 );
}

MoveData Move::get_data() const {
    return this->move_data;
}

bool Move::is_capture() const {
    return static_cast<bool>(this->move_data & Masks::CAPTURE);
}

bool Move::is_promote() const {
    return static_cast<bool>(this->move_data & Masks::PROMOTE);
}

bool Move::is_castle() const {
    const MoveFlag flag = this->get_flag();
    if (flag == Flags::SHORT_CASTLE ||
        flag == Flags::LONG_CASTLE)
    {
        return true;
    }

    return false;
}

bool Move::is_double_pawn_push() const {
    const MoveFlag flag = this->get_flag();
    if (flag == Flags::DOUBLE_PAWN_PUSH) {
        return true;
    }

    return false;
}

bool Move::is_enpassant() const {
    const MoveFlag flag = this->get_flag();
    if (flag == Flags::ENPASSANT) {
        return true;
    }

    return false;
}

bool Move::is_null() const {
    return this->move_data == 0;
}

Square Move::get_from_square() const {
    return static_cast<Square>( (this->move_data & Masks::FROM_SQ) );
}

Square Move::get_to_square() const {
    return static_cast<Square>( (this->move_data & Masks::TO_SQ) >> 6 );
}

PieceType Move::get_promote_piece_type() const {
    if (!this->is_promote()) {return PieceType::NO_PIECE_TYPE;}

    const MoveFlag flag = this->get_flag();
    return static_cast<PieceType>((flag & 0b011) + 1);
}

Castle Move::get_castle() const {
    if (!this->is_castle()) {return CastlingRights::NONE;}

    const MoveFlag flag = this->get_flag();
    if      (flag == Flags::SHORT_CASTLE) {return CastlingRights::W_SHORT_B_SHORT;}
    else if (flag == Flags::LONG_CASTLE)  {return CastlingRights::W_LONG_B_LONG;}

    return CastlingRights::NONE;
}


// OrderedMove score get/set

MoveScore OrderedMove::get_score() const {
    return this->score;
}

void OrderedMove::set_score(MoveScore score) {
    this->score = score;
}


// non-member operators

bool operator< (OrderedMove m1, OrderedMove m2) {
    return m1.get_score() < m2.get_score();
}

bool operator> (OrderedMove m1, OrderedMove m2) {
    return m1.get_score() > m2.get_score();
}

bool operator== (Move m1, Move m2) {
    return m1.get_data() == m2.get_data();
}


// non-member utils

void print_move(Move move, std::ostream& os) {

    const Square from = move.get_from_square();
    const Square to   = move.get_to_square();

    os << FILE_LABELS[file_index(from)]
       << RANK_LABELS[rank_index(from)]
       << FILE_LABELS[file_index(to)]
       << RANK_LABELS[rank_index(to)];

    if (move.is_promote()) {
        os << PIECE_TYPE_LABELS[move.get_promote_piece_type()];
    }

    os << '\n';
}

} // MPChess namespace