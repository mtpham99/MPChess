// tt.hpp

#pragma once

#include "defs.hpp" // types, constants
#include "move.hpp" // move

#include <vector>   // vector
#include <memory>   // unique pointer
#include <atomic>   // atomic


namespace MPChess {

namespace Constants {

inline constexpr std::size_t DEFAULT_TABLE_SIZE_MB = 16;

} // Constants namespace

namespace Types {

struct TTEntry {
    Key      key;   // 8 bytes
    Move     move;  // 2 bytes
    Eval     eval;  // 2 bytes
    Depth    depth; // 2 bytes
    NodeType node;  // 1 byte

    inline bool is_null() const {
        return (this->key   == Key{0}) 
            && (this->node  == NodeType::NULL_NODE)
            && (this->eval  == Eval{0})
            && (this->depth == 0)
            && (this->move.is_null());
    }
};

using AtomicTTEntry    = std::atomic<TTEntry>;
using PtrAtomicTTEntry = std::unique_ptr<AtomicTTEntry>;
inline constexpr std::size_t TTENTRY_SIZE_BYTES = sizeof(AtomicTTEntry)
                                                + sizeof(PtrAtomicTTEntry);

} // Types namespace

class TranspositionTable {
private:

    std::size_t table_size_mb;
    std::size_t table_size;
    
    std::vector<Types::PtrAtomicTTEntry> table;

    std::size_t size; // number of non-null entries
    std::size_t hits; // number of probe hits

public:

    // constructors
    TranspositionTable(std::size_t table_size_mb = Constants::DEFAULT_TABLE_SIZE_MB);


    // resize table
    void resize(std::size_t size_mb);

    // reset
    void reset();

    // store/probe
    Types::TTEntry probe(Types::Key key);
    void store(Types::Key key, Move move, Types::Eval eval, Types::Depth depth, Types::NodeType node);
};

} // MPChess namespace