#include "CNF.h"
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

    void CNF::decrease_usage_count_and_check_if_pure(int p) {
        int ind = std::abs(p) - 1;
        //если после уменьшения счетчика станет pure (и не удалится из кнф) кладем в очередь (хотим класть только 1 раз без повторов)
        if (p < 0) {
            if (variable_sign_usage_count_[ind].minus_ == 1 && variable_sign_usage_count_[ind].plus_ != 0) {
                //стал pure после того как уменьшилось кол-во вхождений с минусом -> pure c +
                possible_pure_queue.push(-p); //знаем что p<0

            }
            variable_sign_usage_count_[ind].minus_--;
        } else {
            if (variable_sign_usage_count_[ind].plus_ == 1 && variable_sign_usage_count_[ind].minus_ != 0) {
                //стал pure после того как уменьшилось кол-во вхождений с плюсом -> pure c -
                possible_pure_queue.push(-p); //знаем что p > 0
            }
            variable_sign_usage_count_[ind].plus_--;
        }
    }

    int CNF::get_usage_count(int p) {
        int ind = std::abs(p) - 1;
        return variable_sign_usage_count_[ind].plus_ + variable_sign_usage_count_[ind].minus_;
    }

    bool CNF::is_pure(int p) {
        int i = std::abs(p) - 1;
        return get_usage_count(p) > 0 && (variable_sign_usage_count_[i].plus_ == 0 ||
                                          variable_sign_usage_count_[i].minus_ == 0);
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

                iss >> p;
            }

            if (tmp_clause.empty()) throw std::invalid_argument(errors::kNotDIMACS);
            clauses_.push_back(std::move(tmp_clause));
            ind++;
        }

        //все переменные входят в кнф?
        for (int var = 1; var <= variables_num_; var++) {
            if (get_usage_count(var) == 0) throw std::invalid_argument(errors::kNotDIMACS);
            if (is_pure(var)) {
                possible_pure_queue.push(var * (variable_sign_usage_count_[var - 1].minus_ == 0 ? 1 : -1));
            }
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
        auto is_unit = [](const Clause &clause) {
            return clause.size() == 1;
        };

        auto unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), is_unit);

        while (unit_clause_iterator != clauses_.end()) { // для всех подходящих unit_clause применяем UnitResolution
            int p = *(unit_clause_iterator->begin());
            model_[std::abs(p)] = p;
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

                if (is_same_sign) { // если наша переменная входит в дизъюнкт с тем же знаком удаляем дизъюнкт
                    for (auto var: *target_clause_iterator) {
                        // уменьшаем число использований у всех переменных содержащихся в дизъюнкте
                        decrease_usage_count_and_check_if_pure(var);
                    }
                    clauses_.erase(target_clause_iterator);
                    clauses_num_--;
                } else { // если переменная входит с другим знаком удаляем её из дизъюнкта
                    decrease_usage_count_and_check_if_pure(*literal_iterator);
                    target_clause_iterator->erase(literal_iterator);
                    if (target_clause_iterator->empty()) {
                        contains_empty_ = true;
                        return;
                    }
                }
                if (get_usage_count(p) == 0) {
                    //обработали все дизъюнкты содержащие p
                    break;
                }
                target_clause_iterator = std::find_if(next_iterator, clauses_.end(),
                                                      contains_p);
            }

            unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(),
                                                is_unit); //FIXME: ultra slow
        }
    }

    //FIXME: возможны "холостые" срабатывания на unit_clausах (один раз макс)
    void CNF::PureLiterals() { // TODO: optimize?
        while (!possible_pure_queue.empty()) {
            int p = possible_pure_queue.front();
            possible_pure_queue.pop();
            // пока проходило исполнение переменная могла перестать быть pure (была удалена из кнф полностью)
            if (is_pure(p)) {
                auto contains_p = [p](Clause &clause) { return clause.find(p) != clause.end(); };
                auto target_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), contains_p);
                while (target_clause_iterator != clauses_.end()) {
                    auto next_iterator = std::next(target_clause_iterator);
                    clauses_num_--;
                    for (auto i: *target_clause_iterator) {
                        decrease_usage_count_and_check_if_pure(i);
                    }
                    clauses_.erase(target_clause_iterator);
                    if (get_usage_count(p) == 0) {
                        //обработали все дизъюнкты содержащие p
                        break;
                    }
                    target_clause_iterator = std::find_if(next_iterator, clauses_.end(), contains_p);
                }
                AddUnitClauseFront(p);
            }
        }
    }

    void CNF::AddUnitClauseFront(int p) {
        std::unordered_set<int> tmp = {p};
        clauses_.emplace_front(std::move(tmp));
        increase_usage_count(p);
        clauses_num_++;
    }

    bool CNF::IsAssigned(int p) const {
        return model_.find(p) != model_.end();
    }

    bool CNF::IsSat() const {
        return clauses_.empty();
    }
}