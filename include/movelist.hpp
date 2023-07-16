// movelist.hpp

#pragma once

#include "defs.hpp" // types, constants
#include "move.hpp" // move

#include <array>    // array

#include <cassert>  // assert


namespace MPChess {

namespace Concepts {

template<typename T>
concept move_like = std::same_as<Move, T> || std::same_as<OrderedMove, T>;

} // Concepts namespace


template<Concepts::move_like move_t>
class MoveList {
private:

    std::size_t                            size = 0;
    std::array<move_t, Constants::MAX_PLY> moves;

public:

    // constructors

    MoveList() = default;


    // getters

    std::size_t get_size() const {
        return this->size;
    }


    // reset/shrink

    void reset() {
        this->size = 0;
        std::fill(this->moves.begin(), this->moves.end(), move_t{});
    }

    void shrink(std::size_t size) {
        if (size > this->size) {assert(false);}
        this->size = size;
    }


    // add moves

    void add_move(move_t move) {
        if (this->size == Constants::MAX_PLY) {
            assert(false);
        }

        this->moves[(this->size)++] = move;
    }

    void add_moves(const MoveList<move_t>& move_list) {
        if (this->size + move_list.get_size() > Constants::MAX_PLY) {
            assert(false);
        }

        std::copy(move_list.begin(), move_list.end(), this->end());
        this->size += move_list.get_size();
    }

    void remove_move(move_t move) {
        auto p_remove_move = std::find(this->begin(), this->end(), move);
        if (p_remove_move != this->end()) {
            std::copy(p_remove_move + 1, this->end(), p_remove_move);
            --(this->size);
        }
    }

    void set_moves(const MoveList<move_t>& move_list) {
        *this = move_list;
    }


    // operators

    const move_t& operator[] (std::size_t index) const {
        if (index >= this->size) {
            assert(false);
        }
        return this->moves[index];
    }

    void operator= (const MoveList<move_t>& rhs) {
        this->size = rhs.size;
        std::copy(rhs.begin(), rhs.end(), this->begin());
    }


    // iterators

    const move_t* begin() const {
        return this->moves.begin();
    }

    move_t* begin() {
        return this->moves.begin();
    }
    
    const move_t* end() const {
        return this->moves.begin() + this->size;
    }

    move_t* end() {
        return this->moves.begin() + this->size;
    }

}; // MoveList class

using OrderedMoveList = MoveList<OrderedMove>;
using RegularMoveList = MoveList<Move>;


// PVLine

class PVLine : public RegularMoveList
{
private:

    Types::Eval score = -Constants::Evals::INF;

public:

    // constructors

    PVLine() = default;
    using RegularMoveList::RegularMoveList;



    // setters

    void set_score(Types::Eval new_score) {
        this->score = new_score;
    }


    // getters

    Types::Eval get_score() const {
        return this->score;
    }


    // operators

    friend bool operator<  (const PVLine& lhs, const PVLine& rhs);
    friend bool operator>  (const PVLine& lhs, const PVLine& rhs);
    friend bool operator== (const PVLine& lhs, const PVLine& rhs);
};


// PVLine operators

inline bool operator< (const PVLine& lhs, const PVLine& rhs) {
    return lhs.score < rhs.score;
}

inline bool operator> (const PVLine& lhs, const PVLine& rhs) {
    return lhs.score > rhs.score;
}

inline bool operator== (const PVLine& lhs, const PVLine& rhs) {
    return lhs.score == rhs.score;
}

} // MPChess namespace
