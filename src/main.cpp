// main.cpp

#include "uci.hpp"
#include "engine.hpp"

auto main() -> int { 

    MPChess::UCI::print_welcome();

    MPChess::UCI::uci_loop();

    return 0;
}