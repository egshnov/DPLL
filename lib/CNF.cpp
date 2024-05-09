#include "CNF.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "errormsg.h"

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
                throw std::invalid_argument(errors::kNotDIMACS);
            }
            iss >> tmp;
            if (tmp != "cnf") {
                throw std::invalid_argument(errors::kNotDIMACS);
            }
            iss >> variables_num_ >> clauses_num_;
            if (variables_num_ <= 0 || clauses_num_ <= 0) {
                throw std::invalid_argument(errors::kNotDIMACS);
            }

        }

        int p, ind = 0;
        variable_sign_usage_count_.resize(variables_num_);
        std::vector<bool> is_used(variables_num_, false);
        while (std::getline(stream, line)) {

            if (ind == clauses_num_) {
                throw std::invalid_argument(errors::kNotDIMACS);
            }

            std::istringstream iss(line); //FIXME: redundant copy
            iss >> p;
            Clause tmp_clause;
            while (p != 0) {
                if (std::abs(p) > variables_num_) {
                    throw std::invalid_argument(errors::kNotDIMACS);
                }
                tmp_clause.insert(p);
                increase_usage_count(p);
                is_used[std::abs(p) - 1] = true;
                iss >> p;
            }

            if (tmp_clause.empty()) throw std::invalid_argument(errors::kNotDIMACS);
            clauses_.push_back(std::move(tmp_clause));
            ind++;
        }

        for (auto used: is_used) {
            if (!used) throw std::invalid_argument(errors::kNotDIMACS);
        }
    }

    void CNF::Parse(const std::string &filename) {
        std::ifstream input(filename);
        if (!input.is_open()) throw std::invalid_argument(errors::kClosed);

        try {
            Init(input);
        }
        catch (const std::string &error_message) {
            throw std::invalid_argument(error_message);
        }
    }

    std::string CNF::ToString() {
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
        //ищем среди дизъюнктов unit_clause такой что его переменная содержится в других дизъюнктах (иначе нет смысла его рассматривать)
        auto is_appropriate_unit = [this](const Clause &clause) {
            return clause.size() == 1 && get_usage_count(*clause.begin()) > 1;
        };
        auto unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), is_appropriate_unit);

        while (unit_clause_iterator != clauses_.end()) { // для всех подходящих unit_clause применяем UnitResolution
            int p = *(unit_clause_iterator->begin());
            bool is_same_sign; // с каким знаком входит в найденный дизъюнкт
            decltype(unit_clause_iterator->begin()) literal_iterator; //итератор из на элемент unordered_set<int>

            auto contains_p = [p, &is_same_sign, &literal_iterator](Clause &clause) {
                literal_iterator = clause.find(p);
                is_same_sign = true;
                if (literal_iterator == clause.end()) {
                    literal_iterator = clause.find(-p);
                    is_same_sign = false;
                }
                return literal_iterator != clause.end();
            };
            // итератор на дизъюнкт содержащий переменную из unit clause
            auto target_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), contains_p);

            while (target_clause_iterator != clauses_.end()) {
                auto next_iterator = std::next(target_clause_iterator); // итератор на элемент следующий за найденным
                if (target_clause_iterator != unit_clause_iterator) { // с нашим unit clause ничего делать не надо


                    if (is_same_sign) { // если наша переменная входит в дизъюнкт с тем же знаком удаляем дизъюнкт
                        for (auto var: *target_clause_iterator) {
                            // уменьшаем число использований у всех переменных содержащихся в дизъюнкте
                            decrease_usage_count(var);
                        }
                        clauses_.erase(target_clause_iterator);
                        clauses_num_--;
                    } else { // если переменная входит с другим знаком удаляем её из дизъюнкта
                        decrease_usage_count(*literal_iterator);
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

            unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(),
                                                is_appropriate_unit); //FIXME: ultra slow
        }
    }

    int CNF::find_pure() { //optimize?
        for (int i = 0; i < variables_num_; i++) {
            if (variable_sign_usage_count_[i].plus_ == 0 ||
                variable_sign_usage_count_[i].minus_ == 0 &&
                get_usage_count(i + 1) > 1) // //входит во все дизъюнкты с одним знаком (и дизъюнктов больше одного)
                return i;
        }
        return -1;
    }

    void CNF::PureLiterals() { // TODO: optimize?
        //std::cout << "Literals" << std::endl;
        int i = find_pure();
        while (i != -1) {
            int sign = variable_sign_usage_count_[i].plus_ != 0 ? 1 : -1;
            int p = (i + 1) * sign;
            std::cout << "current pure " << p << std::endl;
            std::cout << "current cnf" << std::endl << ToString() << std::endl;
            int ct;
            std::cin
                    >> ct;
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
            //std::cout << "after remove" << std::endl << CnfToString() << std::endl;
            clauses_.emplace_front(p);
            increase_usage_count(p);
            //std::cout << "emplaced " << std::endl;
            clauses_num_++;
            i = find_pure();
            //std::cout << "after emplace" << std::endl << CnfToString() << std::endl;
        }
        //std::cout << "literals end" << std::endl;
    }

    void CNF::AddClauseFront(int p) {
        std::unordered_set<int> tmp = {p};
        clauses_.emplace_front(std::move(tmp));
    }

    bool CNF::IsInterpretation() {

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