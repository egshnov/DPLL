#include "CNF.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace solver {
    void CNF::Init(std::ifstream &stream) {
        std::string line, tmp;
        std::getline(stream, line);

        //parse comments
        while (line[0] == 'c') {
            std::getline(stream, line);
        }

        {
            std::istringstream iss(line);

            iss >> tmp;
            if (tmp != "p") {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }
            iss >> tmp;
            if (tmp != "cnf") {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }
            iss >> variables_ >> clauses_;
            if (variables_ <= 0 || clauses_ <= 0) {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }

        }

        storage_.resize(clauses_);
        int p, i = 0;
        while (std::getline(stream, line)) {

            if (i == clauses_) {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }

            std::istringstream iss(line);
            iss >> p;
            while (p != 0) {
                if (std::abs(p) > variables_) {
                    throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
                }
                storage_[i].emplace(p);
                iss >> p;
            }
            i++;
        }
    }

    std::string CNF::CnfToString() {
        std::string result;
        for (int i = 0; i < clauses_; i++) {
            for (auto it: storage_[i]) {
                result += std::to_string(it) + " ";
            }
            result += "0\n";
        }
        return result;
    }
}