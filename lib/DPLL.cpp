#include "DPLL.h"
#include "CNF.h"
#include <vector>
#include <iostream>

namespace solver {

    namespace {
        bool real_solver(CNF &cnf, int branch) {//check branch?
            if (branch > cnf.GetVariablesNum()) return false;
            std::cout << "branch " << branch << std::endl;
            cnf.UnitPropagation();

            if (cnf.ContainsEmpty()) {
                return false;
            }
            std::cout << "after propagation:\n" << cnf.ToString() << std::endl;
            cnf.PureLiterals();
            std::cout << "after pure literals \n" << cnf.ToString() << std::endl;
            if (cnf.IsInterpretation()) return true;
            CNF tmp = cnf;
            tmp.AddClauseFront(branch);
            bool solved = real_solver(tmp, branch + 1);
            if (solved) { //FIXME: redundant copy?
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
