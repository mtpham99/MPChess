// attacks.hpp

#pragma once

#include "defs.hpp"  // types, constants
#include "utils.hpp" // utils (step, shift, square_to_bitboard, rank_bitboard, file_bitboard)

#include "rng.hpp"   // xorshift64

#include <array>     // array
#include <vector>    // vector

#include <iostream>  // cout


namespace MPChess {

namespace Attacks {

// pawn attacks
template<Types::Color c, Concepts::bitboard_like BB_t>
requires (c != Types::Color::NO_COLOR)
constexpr Types::Bitboard pawn_attacks(BB_t pawns) {
    
    // white
    if constexpr (c == Types::Color::WHITE) {
        return step<Types::StepType::NE>(pawns) | step<Types::StepType::NW>(pawns);
    }

    // black
    else {
        return step<Types::StepType::SE>(pawns) | step<Types::StepType::SW>(pawns);
    }

    
}


// knight attacks
template<Concepts::bitboard_like BB_t>
constexpr Types::Bitboard knight_attacks(BB_t knights) {
    const Types::Bitboard attacks = step<Types::StepType::NNE>(knights)
                                  | step<Types::StepType::NEE>(knights)
                                  | step<Types::StepType::SEE>(knights)
                                  | step<Types::StepType::SSE>(knights)
                                  | step<Types::StepType::SSW>(knights)
                                  | step<Types::StepType::SWW>(knights)
                                  | step<Types::StepType::NWW>(knights)
                                  | step<Types::StepType::NNW>(knights);
    return attacks;
}


// king attacks
template<Concepts::bitboard_like BB_t>
constexpr Types::Bitboard king_attacks(BB_t kings) {
    const Types::Bitboard attacks = step<Types::StepType::N>(kings)
                                  | step<Types::StepType::E>(kings)
                                  | step<Types::StepType::S>(kings)
                                  | step<Types::StepType::W>(kings)
                                  | step<Types::StepType::NE>(kings)
                                  | step<Types::StepType::SE>(kings)
                                  | step<Types::StepType::SW>(kings)
                                  | step<Types::StepType::NW>(kings);
    return attacks;
}


// ray attacks
template<Types::StepType direction, Concepts::bitboard_like BB_t>
requires (direction == Types::StepType::N)  || (direction == Types::StepType::E)  || (direction == Types::StepType::S)  || (direction == Types::StepType::W)
      || (direction == Types::StepType::NE) || (direction == Types::StepType::SE) || (direction == Types::StepType::SW) || (direction == Types::StepType::NW)
constexpr Types::Bitboard ray_attacks(Types::Square sq, BB_t occupancy) {
    assert(sq != Types::Square::NO_SQUARE);

    Types::Bitboard sq_bb        = square_to_bitboard(sq);
    
    // shift occupancy to include occupant squares in set of attacks
    Types::Bitboard occupancy_bb;
    if constexpr (std::same_as<Types::Bitboard, BB_t>) {
        occupancy_bb = step<direction>(occupancy);
    }
    else if constexpr (std::same_as<Types::Square, BB_t>) {
        occupancy_bb = square_to_bitboard(step<direction>(occupancy));
    }

    Types::Bitboard attacks{Constants::EMPTY};
    while (!((sq_bb = step<direction>(sq_bb)) & occupancy_bb) // stop if previous square was occupant
           && sq_bb)                                          // stop if stepped "off" board
    {
        attacks |= sq_bb;
    }

    return attacks;
}


// slider attacks
template<Types::PieceType slider, Concepts::bitboard_like BB_t>
requires (slider == Types::PieceType::BISHOP) || (slider == Types::PieceType::ROOK) || (slider == Types::PieceType::QUEEN)
constexpr Types::Bitboard slider_attacks(Types::Square sq, BB_t occupancy) {
    assert(sq != Types::Square::NO_SQUARE);

    if constexpr (slider == Types::PieceType::QUEEN) {
        return slider_attacks<Types::PieceType::BISHOP>(sq, occupancy)
             | slider_attacks<Types::PieceType::ROOK>(sq, occupancy);
    }
                                                                             // bishop             // rook
    constexpr Types::StepType ray_1 = (slider == Types::PieceType::BISHOP) ? Types::StepType::NE : Types::StepType::N;
    constexpr Types::StepType ray_2 = (slider == Types::PieceType::BISHOP) ? Types::StepType::SE : Types::StepType::E;
    constexpr Types::StepType ray_3 = (slider == Types::PieceType::BISHOP) ? Types::StepType::SW : Types::StepType::S;
    constexpr Types::StepType ray_4 = (slider == Types::PieceType::BISHOP) ? Types::StepType::NW : Types::StepType::W;
 
    const Types::Bitboard attacks = ray_attacks<ray_1>(sq, occupancy) | ray_attacks<ray_2>(sq, occupancy)
                                  | ray_attacks<ray_3>(sq, occupancy) | ray_attacks<ray_4>(sq, occupancy);

    return attacks;
}


// relevant blocker mask
template<Types::PieceType slider>
requires (slider == Types::PieceType::BISHOP) || (slider == Types::PieceType::ROOK)
constexpr Types::Bitboard relevant_blocker_mask(Types::Square sq) {
    assert(sq != Types::Square::NO_SQUARE);
    
    // irrelevant blocker squares are edges of board
    // because nothing can be "behind" a piece on the edge
    // for a bishop the irrelevant squares are always all 4 edges of board
    // for a rook, the irrelevant squares are the edges of the rank/file the rook is on
    // e.g. irrelevant blocker squares for rook on a1 are a8 and h1
    // e.g. irrelevant blocker squares for rook on a4 are a8, a1, and h4

    const Types::Bitboard irrelevant_edges = ((Constants::RANK_1 | Constants::RANK_8) & ~rank_bitboard(sq))
                                           | ((Constants::FILE_A | Constants::FILE_H) & ~file_bitboard(sq));

    const Types::Bitboard attacks_on_empty = slider_attacks<slider>(sq, Constants::EMPTY);

    return attacks_on_empty & ~irrelevant_edges;
}


namespace Magics {

struct MagicEntry {
    Types::Bitboard blockers_mask;
    uint64_t        magic;
    std::size_t     key_shift;
    std::size_t     table_offset;

    inline std::size_t get_index(Types::Bitboard occupancy) const {
        occupancy &= this->blockers_mask;
        const int key = (this->magic * occupancy) >> this->key_shift;
        return key + this->table_offset;
    }
};

using MagicTable = std::vector<MagicEntry>;


// find magics
template<Types::PieceType slider, bool optimize=false>
requires (slider == Types::PieceType::BISHOP) || (slider == Types::PieceType::ROOK)
MagicEntry find_magic(Types::Square sq, 
                      std::vector<Types::Bitboard>* const p_attack_table=nullptr,
                      std::size_t iterations=100000000,
                      Rng::XorShift64& rng=Rng::main_rng) {

    // get relevant blocker squares and shift used for hashing
    const Types::Bitboard relevant_blockers = relevant_blocker_mask<slider>(sq);
    const std::size_t     bit_count         = pop_count(relevant_blockers);
    std::size_t           key_shift         = UINT64_WIDTH - bit_count;

    // find magics using brute force
    //
    // try a magic by generating a random, sparse (not many 1 bits) 64-bit number
    // a magic is valid if it correctly maps the attacks of all possible blocker
    // configurations for the sliding piece on a given square
    // the relevant blocker squares are all the squares a blocker can be on
    // to validate a magic is valid, need to generate every blocker combination/subset
    // and use the magic number to hash the subset
    // if the magic number can hash each subset without any destructive collisions
    // then the magic is valid
    //
    // a destructive collision is when a magic hashes multiple blocker
    // combinations to the same hash key when the block combinations have
    // different attack sets
    // 
    // a constructive collision is okay i.e. when the magic hashes multiple different
    // blocker combinations to the same hash key, but the blocker combinations
    // have the same attack set (e.g. a bishop on a1 will have the same attack
    // set if there are blockers on [e4, f4, g6] or [e4, g6] or [e4, h8] or ...)
    //
    // the hashing function is (magic_bitboard * blockers_bitboard) >> (64 - N)
    // where N is the number of bits used for a hash key
    // it is easy to find magic numbers for N = number of blockers in the relevant blocker mask
    // it is possible to find some magics for some squares where N < number of blockers in relevant mask
    // but computationally it is much harder
    // a smaller N just means a smaller final lookup table

    // first generate all possible blocker subsets and corresponding attack sets
    // will generate subsets using "Carry-Ripple" iteration

    std::vector<Types::Bitboard> blocker_subsets;
    blocker_subsets.reserve(1 << bit_count);

    std::vector<Types::Bitboard> attack_subsets;
    attack_subsets.reserve(1 << bit_count);

    // carry-ripple iteration
    Types::Bitboard blocker_subset{Constants::EMPTY};
    do {
        blocker_subset = (blocker_subset - relevant_blockers) & relevant_blockers;
        const Types::Bitboard attacks = slider_attacks<slider>(sq, blocker_subset);

        blocker_subsets.push_back(blocker_subset);
        attack_subsets.push_back(attacks);

    } while(blocker_subset);

    // hash function
    auto hasher = [](uint64_t magic, Types::Bitboard blockers, std::size_t key_shift) -> int {
        return (magic * blockers) >> key_shift;
    };

    // find magics
    Types::Bitboard    best_magic{Constants::EMPTY};
    std::size_t best_hash_size = (1 << bit_count) + 1;

    // mapped attacks
    // initialize to empty (no piece/square/blocker combo has an empty attack set)
    std::vector<Types::Bitboard> mapped_attacks(1 << bit_count, Constants::EMPTY);

    bool repeat_magic = false; // used to repeat best_magic with larger key_shift
    std::size_t iter=0;
    while (iter++ <= iterations) {

        // generate candidate magic
        Types::Bitboard magic = (!repeat_magic) ? rng.generate<Rng::RngType::SPARSE>() : best_magic;

        bool invalid_magic = false;
        std::size_t hash_size = 0;

        // reset mapped attacks
        std::fill(mapped_attacks.begin(), mapped_attacks.end(), Constants::EMPTY);

        // loop over all blocker subsets and hash
        // make sure no destructive collisions
        for (std::size_t i=0; i<blocker_subsets.size(); ++i) {

            const int hash_key = hasher(magic, blocker_subsets[i], key_shift);

            // unmapped attacks
            if (mapped_attacks[hash_key] == Constants::EMPTY) {

                mapped_attacks[hash_key] = attack_subsets[i];
                ++hash_size;

                // if current hash size is larger then the best hash size so far,
                // restart to try and find magic with more collisions
                if (hash_size > best_hash_size) {
                    invalid_magic = true;
                    break;
                }
            }
            // constructive collision
            else if (mapped_attacks[hash_key] == attack_subsets[i]) {

            }
            // destructive collision -- start over with new magic candidate
            else {
                invalid_magic = true;
                break;
            }
        }

        // if magic found
        if (!invalid_magic) {

            if constexpr (optimize) {
                std::cout << "\tIter: " << iter << " / " << iterations << "\n";
                std::cout << "\tFound magic with hash size: " << hash_size << "\n";
                std::cout << "\tMagic: " << magic << "\n";
                std::cout << "\tPiece/Types::Square: " << Constants::PIECE_LABELS[slider] << " / " << square_to_notation(sq) << "\n\n";
            }

            // if better than previous magic
            if (hash_size < best_hash_size) {

                // update best magic
                best_magic = magic;
                best_hash_size = hash_size;

                // if optimization flag not set just return first found valid magic
                if constexpr (!optimize) {
                    break;
                }

                // revalidate magic using larger key_shift
                // TODO : maybe only do this if hash_size is near half 2^(bit_count)
                repeat_magic = true;
                ++key_shift;
                --iter;
            }
        }

        // if failed to revalidate with bigger key shift
        if (repeat_magic) {
            repeat_magic = false;
            
            // change key_shift back to last valid size
            if (invalid_magic) {
                --key_shift;
            }
        }
    }

    std::size_t table_offset = 0;
    if (p_attack_table != nullptr) {

        table_offset = p_attack_table->size();

        p_attack_table->insert(p_attack_table->end(),
                               mapped_attacks.begin(),
                               mapped_attacks.end());
    }

    return {relevant_blockers, best_magic, key_shift, table_offset};
}


// generate magics
template<Types::PieceType slider>
requires (slider == Types::PieceType::BISHOP) || (slider == Types::PieceType::ROOK)
MagicTable generate_magics() {

    MagicTable magic_table(Constants::NUM_SQUARES);
    std::size_t offset = 0; // offset each square by hash size of previous squares
    for (const Types::Square& sq : Constants::ALL_SQUARES) {
        
        MagicEntry magic_entry = Magics::find_magic<slider>(sq);
        magic_entry.table_offset = offset;

        const std::size_t key_size  = UINT64_WIDTH - magic_entry.key_shift;
        const std::size_t hash_size = (1 << key_size);
        offset += hash_size;

        magic_table[sq] = magic_entry;
    }

    return magic_table;
}

} // Magics namespace


namespace Tables {

inline Magics::MagicTable BISHOP_MAGICS = Magics::generate_magics<Types::PieceType::BISHOP>();
inline Magics::MagicTable ROOK_MAGICS   = Magics::generate_magics<Types::PieceType::ROOK>();

// generate sliding attack table
template<Types::PieceType slider>
requires (slider == Types::PieceType::BISHOP) || (slider == Types::PieceType::ROOK)
std::vector<Types::Bitboard> generate_sliding_attack_table(const Magics::MagicTable& magic_table) {

    // calculate table size
    std::size_t table_size = 0;
    for (const Magics::MagicEntry& magic_entry : magic_table) {
        
        const std::size_t key_size  = (UINT64_WIDTH - magic_entry.key_shift);
        const std::size_t hash_size = (1 << key_size);
        table_size += hash_size;
    }

    // generate attacks for each blocker subset
    std::vector<Types::Bitboard> attack_table(table_size);
    for (const Types::Square& sq : Constants::ALL_SQUARES) {

        const auto& [relevant_blockers,
                     magic,
                     key_shift,
                     table_offset] = magic_table[sq];

        // iterate over all blocker subsets
        Types::Bitboard blocker_subset{Constants::EMPTY};
        do {

            blocker_subset = (blocker_subset - relevant_blockers) & relevant_blockers;

            // generate attacks for blocker subset
            const Types::Bitboard attacks = slider_attacks<slider>(sq, blocker_subset);
            const int hash_key = (magic * blocker_subset) >> key_shift;

            // store in table
            attack_table[hash_key + table_offset] = attacks;
        } while (blocker_subset);
    }

    return attack_table;
}

// pawn attack table
inline constexpr std::array<std::array<Types::Bitboard, Constants::NUM_SQUARES>, Constants::NUM_COLORS> PAWN_ATTACK_TABLE = [] consteval {

    std::array<std::array<Types::Bitboard, Constants::NUM_SQUARES>, Constants::NUM_COLORS> attacks;
    
    for (const Types::Square& sq : Constants::ALL_SQUARES) {

        attacks[Types::Color::WHITE][sq] = pawn_attacks<Types::Color::WHITE>(sq);
        attacks[Types::Color::BLACK][sq] = pawn_attacks<Types::Color::BLACK>(sq);
    }

    return attacks;
}();

// knight attack table
inline constexpr std::array<Types::Bitboard, Constants::NUM_SQUARES> KNIGHT_ATTACK_TABLE = [] consteval {

    std::array<Types::Bitboard, Constants::NUM_SQUARES> attacks;

    for (const Types::Square& sq : Constants::ALL_SQUARES) {
        attacks[sq] = knight_attacks(sq);
    }

    return attacks;
}();

// king attack table
inline constexpr std::array<Types::Bitboard, Constants::NUM_SQUARES> KING_ATTACK_TABLE = [] consteval {

    std::array<Types::Bitboard, Constants::NUM_SQUARES> attacks;

    for (const Types::Square& sq : Constants::ALL_SQUARES) {
        attacks[sq] = king_attacks(sq);
    }

    return attacks;
}();

// bishop attack table
inline std::vector<Types::Bitboard> BISHOP_ATTACK_TABLE = generate_sliding_attack_table<Types::PieceType::BISHOP>(BISHOP_MAGICS);

// rook attack table
inline std::vector<Types::Bitboard> ROOK_ATTACK_TABLE = generate_sliding_attack_table<Types::PieceType::ROOK>(ROOK_MAGICS);

// inbetween squares table
inline constexpr std::array<std::array<Types::Bitboard, Constants::NUM_SQUARES>, Constants::NUM_SQUARES> INBETWEEN_SQUARES_TABLE = [] consteval {

    std::array<std::array<Types::Bitboard, Constants::NUM_SQUARES>, Constants::NUM_SQUARES> inbetween_squares;

    // squares are on same line (diag/row/col) if bishop/rook
    // on square_1 attacks square_2
    //
    // if squares are on same line
    // then inbetween squares are overlap of bishop/rook attacks
    // from each square
    for (const Types::Square& sq_1 : Constants::ALL_SQUARES) {
        for (const Types::Square& sq_2 : Constants::ALL_SQUARES) {

            const Types::Bitboard diag_overlap   = (slider_attacks<Types::PieceType::BISHOP>(sq_1, Constants::EMPTY) & square_to_bitboard(sq_2)) ? slider_attacks<Types::PieceType::BISHOP>(sq_1, sq_2) & slider_attacks<Types::PieceType::BISHOP>(sq_2, sq_1) : Constants::EMPTY;
            const Types::Bitboard rowcol_overlap = (slider_attacks<Types::PieceType::ROOK>(sq_1, Constants::EMPTY) & square_to_bitboard(sq_2)) ? slider_attacks<Types::PieceType::ROOK>(sq_1, sq_2) & slider_attacks<Types::PieceType::ROOK>(sq_2, sq_1) : Constants::EMPTY;

            inbetween_squares[sq_1][sq_2] = diag_overlap | rowcol_overlap;
            inbetween_squares[sq_2][sq_1] = inbetween_squares[sq_1][sq_2];
        }
    }

    return inbetween_squares;
}();

} // Tables namespace


// slider attacks lookup
template<Types::PieceType pt, Concepts::bitboard_like BB_t>
requires (pt == Types::PieceType::BISHOP) || (pt == Types::PieceType::ROOK) || (pt == Types::PieceType::QUEEN)
Types::Bitboard slider_attacks_lookup(BB_t pieces, Types::Bitboard occupancy) {

    if constexpr (pt == Types::PieceType::QUEEN) {
        return slider_attacks_lookup<Types::PieceType::BISHOP>(pieces, occupancy)
             | slider_attacks_lookup<Types::PieceType::ROOK>(pieces, occupancy);
    }

    // square input (lookup)
    if constexpr (std::same_as<Types::Square, BB_t>) {
        const Magics::MagicEntry& magic_entry = (pt == Types::PieceType::BISHOP) ? Tables::BISHOP_MAGICS[pieces]
                                                                                 : Tables::ROOK_MAGICS[pieces];

        const std::size_t index = magic_entry.get_index(occupancy);

        return (pt == Types::PieceType::BISHOP) ? Tables::BISHOP_ATTACK_TABLE[index]
                                                : Tables::ROOK_ATTACK_TABLE[index];
    }

    // bitboard input (iterate over squares lookup)
    else if constexpr (std::same_as<Types::Bitboard, BB_t>) {

        Types::Bitboard attacks = Constants::EMPTY;
        while(pieces) {
            const Types::Square sq = pop_lsb(pieces);
            attacks |= slider_attacks_lookup<pt>(sq, occupancy);
        }

        return attacks;
    }
}


// general attack table lookup
template<Types::PieceType pt, Types::Color c=Types::Color::NO_COLOR, Concepts::bitboard_like BB_t>
Types::Bitboard attacks(BB_t pieces, Types::Bitboard occupancy=Constants::EMPTY) {
    if constexpr (pt == Types::PieceType::NO_PIECE_TYPE) {return Constants::EMPTY;}

    if constexpr (pt == Types::PieceType::PAWN) {
        return pawn_attacks<c>(pieces);
    }
    else if constexpr (pt == Types::PieceType::KNIGHT) {
        return knight_attacks(pieces);
    }
    else if constexpr (pt == Types::PieceType::BISHOP ||
                       pt == Types::PieceType::ROOK   ||
                       pt == Types::PieceType::QUEEN) {
        return slider_attacks_lookup<pt>(pieces, occupancy);
    }
    else if constexpr (pt == Types::PieceType::KING) {
        return king_attacks(pieces);
    }
}


// inbetween squares lookup
constexpr Types::Bitboard inbetween_squares(Types::Square sq_1, Types::Square sq_2) {
    return Tables::INBETWEEN_SQUARES_TABLE[sq_1][sq_2];
}


} // Attacks namespace

} // MPChess namespace