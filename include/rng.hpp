// rng.hpp

#pragma once

#include <cstdint> // fixed width types
#include <array>   // array

#include <cassert> // assert

namespace MPChess {

namespace Rng {

    inline constexpr uint64_t DEFAULT_SEED = 84629465829ull;
    
    // generate number type
    enum RngType {NORMAL, SPARSE};


    // 64bit XorShifter
    // https://en.wikipedia.org/wiki/Xorshift#Example_implementation

    class XorShift64 {
    private:

        uint64_t state;

    public:

        // constructors
        XorShift64() : state{Rng::DEFAULT_SEED} {

        }

        XorShift64(uint64_t initial_state) : state{initial_state} {
            assert(this->state);
        }
        

        // no copy
        XorShift64(const XorShift64& rng)             = delete;
        XorShift64& operator= (const XorShift64& rng) = delete;


        // generate pseudo random numbers

        template<RngType rng_type = RngType::NORMAL>
        uint64_t generate() {

            // normal rng
            if constexpr (rng_type == RngType::NORMAL) {
                this->state ^= this->state >> 12;
                this->state ^= this->state << 25;
                this->state ^= this->state >> 27;
                return this->state * 0x2545f4914f6cdd1dull;
            }

            // sparse rng (only a few set bits/1's)
            else if constexpr (rng_type == RngType::SPARSE) {
                return this->generate() & this->generate() & this->generate(); 
            }
        }

        template<std::size_t N, RngType rng_type = RngType::NORMAL>
        std::array<uint64_t, N> generate_N() {

            std::array<uint64_t, N> pseudo_random_nums;
            for (auto& num : pseudo_random_nums) {
                num = this->generate<rng_type>();
            }
            return pseudo_random_nums;
        }

    }; // XorShift64 class


    // global default rng
    inline XorShift64 main_rng;

} // Rng namespace

} // MPChess namespace