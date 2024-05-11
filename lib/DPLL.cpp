#include "DPLL.h"
#include "CNF.h"
#include <iostream> //TODO: rm

namespace solver {

    namespace {

        bool real_solver(CNF &cnf, int next_to_assign) {
            cnf.UnitPropagation();

            if (cnf.IsUnsat()) {
                return false;
            }

            //TODO: могли вычистить неприсвоенные
            if (cnf.IsInterpretation()) return true; // т.к. только UnitPropagation присваивает значение и вычищает кнф
            cnf.PureLiterals();

            while (cnf.IsAssigned(next_to_assign)) {
                if (next_to_assign > cnf.GetVariablesNum()) {
                    return false;
                }
                next_to_assign++;
            }

            CNF tmp = cnf;
            tmp.AddUnitClauseFront(next_to_assign);
            bool solved = real_solver(tmp, next_to_assign + 1);

            if (solved) {
                cnf = tmp;
                return true;
            }

            tmp = cnf;
            tmp.AddUnitClauseFront(-next_to_assign);
            solved = real_solver(tmp, next_to_assign + 1);
            if (solved) {
                cnf = tmp;
            }
            return solved;
        }
    }

    bool DPLL(CNF &cnf) {
        CNF tmp = cnf;
        bool solved = real_solver(tmp, 1);

        if (solved) {
            cnf = tmp;
        }

        return solved;
    }
}
