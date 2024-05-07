#include "DPLL.h"
#include "CNF.h"
#include <vector>

namespace solver {

    namespace {
        bool real_solver(CNF &cnf, int branch) { //check branch?
            cnf.UnitPropagation();
            if (cnf.ContainsEmpty()) {
                return false;
            }
            cnf.PureLiterals();
            if (cnf.IsInterpetation()) return true;
            cnf.AddClauseFront(branch);
            CNF tmp = cnf;
            tmp.AddClauseFront(branch);
            bool solved = real_solver(tmp, branch + 1);
            if (solved) {
                cnf = tmp;
                return true;
            }
            cnf.AddClauseFront(-branch);
            return real_solver(cnf, branch + 1);
        }
    }

    bool DPLL(CNF &cnf) {
        bool solved = real_solver(cnf, 1);
        return solved;
    }
}
