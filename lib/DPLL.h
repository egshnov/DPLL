#pragma once

#include <vector>
#include "CNF.h"

namespace solver {
    // 2 структуры литералов: pure и не pure
    // 2 структуры clause: unit clause и обычный
    //returns empty vector if CNF is UNSAT
    bool DPLL(CNF &cnf);
}
