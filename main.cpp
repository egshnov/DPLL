#include <iostream>
#include <benchmark/benchmark.h>
#include "CNF.h"
#include "DPLL.h"

void func_to_bench() {
    solver::CNF cnf;

    try {
        cnf.Parse("../test/test_input_data/satlib.cnf");
    }
    catch (const std::string &error_message) {
        std::cerr << error_message << std::endl;
    }

    bool solved = solver::DPLL(cnf);

    if (!solved) {
        std::cout << "s UNSATISFIABLE" << std::endl;
    } else {
        std::cout << "s SATISFIABLE\nv ";
        for (int i = 0; i < cnf.GetVariablesNum(); i++) {
            int sign = cnf.GetModel()[i];
            if (sign == 0) {
                std::cout << i + 1 << " ";
            } else {
                std::cout << (i + 1) * sign << " ";
            }
        }
        std::cout << "0" << std::endl;
    }
}

static void BM_DPLL(benchmark::State &state) {
    for (auto _: state) {
        func_to_bench();
    }
}

BENCHMARK(BM_DPLL)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();
