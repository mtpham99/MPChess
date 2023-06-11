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

    std::size_t                             size;
    std::array<move_t, Constants::MAX_PLY> moves;

public:

    // constructors
    MoveList() : size{0} {}


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

        this->moves[this->size] = move;
        ++(this->size);
        // this->moves[(this->size)++] = move;
    }

    void add_moves(const MoveList<move_t>& move_list) {
        if (this->size + move_list.get_size() > Constants::MAX_PLY) {
            assert(false);
        }

        std::copy(move_list.begin(), move_list.end(), this->end());
        this->size += move_list.get_size();
    }


    // operators
    const move_t& operator[] (std::size_t index) const {
        if (index >= this->size) {
            assert(false);
        }
        return this->moves[index];
    }


    // iterators
    const move_t* begin() const {
        return this->moves.begin();
    }
    
    const move_t* end() const {
        return this->moves.begin() + this->size;
    }

}; // MoveList class

namespace Types {

using OrderedMoveList = MoveList<OrderedMove>;
using RegularMoveList = MoveList<Move>;

} // Types namespace

} // MPChess namespace
