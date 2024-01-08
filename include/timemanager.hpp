// timemanager.hpp

#include "defs.hpp"       // types, constants


namespace MPChess {

// forward declarations
class  Board;
struct SearchInfo;

Types::Milliseconds calculate_search_time(const Board& board,
                                          Types::Milliseconds white_time,
                                          Types::Milliseconds black_time,
                                          Types::Milliseconds white_inc,
                                          Types::Milliseconds black_inc,
                                          std::size_t moves_to_go);

Types::Milliseconds calculate_search_time(const Board& board, const SearchInfo& search_info);

} // MPChess namespace