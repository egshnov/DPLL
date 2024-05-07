#include "CNF.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

namespace solver {
    void CNF::increase_usage_count(int p) {
        int ind = std::abs(p) - 1;
        if (p < 0) {
            variable_sign_usage_count_[ind].minus_++;
        } else {
            variable_sign_usage_count_[ind].plus_++;
        }
    }

    void CNF::decrease_usage_count(int p) {
        int ind = std::abs(p) - 1;
        if (p < 0) {
            variable_sign_usage_count_[ind].minus_--;
        } else {
            variable_sign_usage_count_[ind].plus_--;
        }
    }

    int CNF::get_usage_count(int p) {
        int ind = std::abs(p) - 1;
        return variable_sign_usage_count_[ind].plus_ + variable_sign_usage_count_[ind].minus_;
    }

    void CNF::Init(std::ifstream &stream) {
        std::string line, tmp;
        std::getline(stream, line);

        //parse comments
        while (line[0] == 'c') {
            std::getline(stream, line);
        }

        {
            std::istringstream iss(line); //FIXME: redundant copy

            iss >> tmp;
            if (tmp != "p") {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }
            iss >> tmp;
            if (tmp != "cnf") {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }
            iss >> variables_num_ >> clauses_num_;
            if (variables_num_ <= 0 || clauses_num_ <= 0) {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }

        }

        int p, ind = 0;
        variable_sign_usage_count_.resize(variables_num_);
        std::vector<bool> is_used(variables_num_, false);
        while (std::getline(stream, line)) {

            if (ind == clauses_num_) {
                throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            }

            std::istringstream iss(line); //FIXME: redundant copy
            iss >> p;
            Clause tmp_clause;
            while (p != 0) {
                if (std::abs(p) > variables_num_) {
                    throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
                }
                tmp_clause.insert(p);
                increase_usage_count(p);
                is_used[std::abs(p) - 1] = true;
                iss >> p;
            }

            if (tmp_clause.empty()) throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
            clauses_.push_back(std::move(tmp_clause));
            ind++;
        }

        for (auto used: is_used) {
            if (!used) throw std::invalid_argument("Given file doesn't satisfy DIMACS format");
        }
    }

    std::string CNF::CnfToString() {
        std::string result;
        for (const auto &c: clauses_) {
            for (auto it: c) {
                result += std::to_string(it) + " ";
            }
            result += "0\n";
        }
        return result;
    }

    void CNF::UnitPropagation() { //TODO: split?

        auto is_appropriate_unit = [this](const Clause &clause) {
            return clause.size() == 1 && get_usage_count(*clause.begin()) > 1;
            //если unit и есть смысл проверять наличие переменной в других дизъюнктах.
        };
        auto unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), is_appropriate_unit);

        while (unit_clause_iterator != clauses_.end()) { // для всех unit_clause применяем UnitPropagation (пока есть)
            int p = *(unit_clause_iterator->begin());
            bool is_same_sign; // с каким знаком входит в найденный дизъюнкт
            decltype(unit_clause_iterator->begin()) literal_iterator;

            auto contains_p = [p, &is_same_sign, &literal_iterator](Clause &clause) {
                literal_iterator = clause.find(p);
                is_same_sign = true;
                if (literal_iterator == clause.end()) {
                    literal_iterator = clause.find(-p);
                    is_same_sign = false;
                }
                return literal_iterator != clause.end();
            };

            auto target_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), contains_p);
            while (target_clause_iterator != clauses_.end()) {
                auto next_iterator = std::next(target_clause_iterator);
                if (target_clause_iterator != unit_clause_iterator) {

                    decrease_usage_count(*literal_iterator);

                    if (is_same_sign) {
                        clauses_.erase(target_clause_iterator);
                        clauses_num_--;
                    } else {
                        target_clause_iterator->erase(literal_iterator);
                        if (target_clause_iterator->empty()) {
                            contains_empty_ = true;
                            return;
                        }
                    }
                }
                target_clause_iterator = std::find_if(next_iterator, clauses_.end(),
                                                      contains_p);
            }

            unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), is_appropriate_unit);
        }
    }

    int CNF::find_pure() { //optimize?
        for (int i = 0; i < variables_num_; i++) {
            if (variable_sign_usage_count_[i].plus_ == 0 ||
                variable_sign_usage_count_[i].minus_ == 0 &&
                get_usage_count(i) > 1) // //входит во все дизъюнкты с одним знаком (и дизъюнктов больше одного)
                return i;
        }
        return -1;
    }

    void CNF::PureLiterals() { // TODO: optimize?
        int i = find_pure();
        while (i != -1) {

            int sign = variable_sign_usage_count_[i].plus_ != 0 ? 1 : -1;
            int p = (i + 1) * sign;

            auto contains_p = [this, p](Clause &clause) {
                bool contains = clause.find(p) != clause.end();
                if (contains) {
                    clauses_num_--;
                    for (auto i: clause) {
                        decrease_usage_count(i);
                    }
                }
                return contains;
            };

            std::remove_if(clauses_.begin(), clauses_.end(), contains_p);
            clauses_.emplace_front(p);
            clauses_num_++;
            i = find_pure();
        }
    }

    void CNF::AddClauseFront(int p) {
        clauses_.emplace_front(p);
    }

    bool CNF::IsInterpetation() {

        if (clauses_num_ != variables_num_) {
            return false;
        }
        std::vector<bool> is_used(variables_num_, false);
        for (auto const &it: clauses_) {
            if (it.size() != 1 || is_used[std::abs(*it.begin()) - 1]) return false;
            is_used[std::abs(*it.begin()) - 1] = true;
        }
        return true;
    }
}