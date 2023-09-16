// search.hpp

#pragma once

#include "defs.hpp"     // types, constants

#include "movelist.hpp" // movelist


namespace MPChess {

// Forward declarations
class  EngineThread;

// EngineThread friend search functions

Types::Eval search(EngineThread& thread);
Types::Eval alpha_beta(EngineThread& thread, Types::Depth depth, Types::Eval alpha, Types::Eval beta, bool root, RegularMoveList& pv_parent);
Types::Eval quiescence(EngineThread& thread, Types::Eval alpha, Types::Eval beta);

} // MPChess namespace