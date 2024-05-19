#include "DPLL.h"
#include "CNF.h"

namespace solver {

    namespace {

        bool real_solver(CNF &cnf, int next_to_assign) {
            bool can_simlify = true;
            while (can_simlify) {
                cnf.UnitPropagation();
                if (cnf.IsUnsat()) {
                    return false;
                }

                if (cnf.IsSat()) return true; // т.к. только UnitPropagation присваивает значение и вычищает кнф

                can_simlify = cnf.PureLiterals();
            }

            //выбираем переменную которой будет присвоено значение
            while (cnf.IsAssigned(next_to_assign)) {
                //найдется т.к. модель не пустая
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
