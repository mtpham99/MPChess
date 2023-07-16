// uci.hpp

#pragma once

#include <string>

#include <iostream>   // cout
#include <syncstream> // osyncstream
#include <sstream>    // stringstream


namespace MPChess {

// Forward declaration
class Move;
class Board;


namespace UCI {

inline std::osyncstream sync_out{std::cout};

// utils

std::string move_to_uci_notation(Move move);
Move uci_notation_to_move(std::string_view notation, const Board& board);


// uci specification

void print_welcome();
void uci_loop();

void parse_position(std::istringstream& stream);
void parse_go(std::istringstream& stream);

} // UCI namespace

} // MPChess namespace