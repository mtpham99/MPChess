// transpositiontable.cpp

#include "tt.hpp"

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

// constructor
TranspositionTable::TranspositionTable(std::size_t table_size_mb) :
    table_size_mb{table_size_mb},
    table_size{(table_size_mb * 1024 * 1024) / TTENTRY_SIZE_BYTES},
    table(table_size)
{
    for (auto& p_entry : this->table) {
        p_entry = std::make_unique<AtomicTTEntry>();
    }
}


// resize/reset
void TranspositionTable::resize(std::size_t size_mb) {
    this->table_size_mb = size_mb;
    this->table_size   = (size_mb * 1024 * 1024) / TTENTRY_SIZE_BYTES;
    this->table.resize(this->table_size);

    for (auto& p_entry : this->table) {
        p_entry = std::make_unique<AtomicTTEntry>();
    }
}

void TranspositionTable::reset() {
    for (auto& p_entry : this->table) {
        p_entry = std::make_unique<AtomicTTEntry>();
    }

    this->hits = 0;
    this->size = 0;
}


// store/probe
TTEntry TranspositionTable::probe(Key key) {
    const std::size_t index = key % this->table_size;

    const TTEntry entry = this->table[index]->load(std::memory_order_relaxed);
    if (key == entry.key) {
        ++(this->hits);
        return entry;
    }

    return {}; // return default/null entry
}

void TranspositionTable::store(Key key, Move move, Eval eval, uint16_t depth, NodeType node) {
    const std::size_t index = key % this->table_size;

    const TTEntry current_entry = this->table[index]->load(std::memory_order_relaxed);
    const TTEntry new_entry     = {key, move, eval, depth, node};

    if (current_entry.is_null()) {++(this->size);}

    this->table[index]->store(new_entry);
}

} // MPChess namespace