#pragma once

#include <unordered_set>
#include <string>
#include <fstream>
#include <vector>

namespace solver {
    class CNF {
    private:
        using Clause = std::unordered_set<int>;

        std::vector<Clause> storage_;
        int variables_;
        int clauses_;

    public:
        std::string CnfToString();

       void Init(std::ifstream &stream);

        void UnitPropogation();

        void PureLiterals();
    };
}
