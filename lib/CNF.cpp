#include "CNF.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "errormsg.h"
#include <queue>

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
                possible_pure_queue_.push(-p); //знаем что p < 0

            }
            variable_sign_usage_count_[ind].minus_--;
        } else {
            if (variable_sign_usage_count_[ind].plus_ == 1 && variable_sign_usage_count_[ind].minus_ != 0) {
                //стал pure после того как уменьшилось кол-во вхождений с плюсом -> pure c -
                possible_pure_queue_.push(-p); //знаем что p > 0
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
        model_.resize(variables_num_, 0);
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

                //если в строке повторяются переменные с одним знаком например -1 -1 0
                 if (tmp_clause.find(p) == tmp_clause.end()) {
                    increase_usage_count(p);
                }

                tmp_clause.insert(p);

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
                possible_pure_queue_.push(var * (variable_sign_usage_count_[var - 1].minus_ == 0 ? 1 : -1));
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

    void CNF::UnitPropagation() {
        //находим все unit_clause и кладем в очередь те чьи переменные попались в первый раз (иначе инвалидируется итератор)
        std::queue<decltype(clauses_.begin())> units_queue;
        std::vector<bool> variable_is_in_queue(variables_num_, false);

        auto is_appropriate_unit = [&variable_is_in_queue](const Clause &clause) {
            if (clause.size() == 1) {
                int ind = std::abs(*clause.begin()) - 1;
                if (!variable_is_in_queue[ind]) {
                    variable_is_in_queue[ind] = true;
                    return true;
                }
            }
            return false;
        };

        auto unit_clause_iterator = std::find_if(clauses_.begin(), clauses_.end(), is_appropriate_unit);
        while (unit_clause_iterator != clauses_.end()) {
            auto next_iterator = std::next(unit_clause_iterator);
            units_queue.push(unit_clause_iterator);
            unit_clause_iterator = std::find_if(next_iterator, clauses_.end(), is_appropriate_unit);
        }


        while (!units_queue.empty()) {
            unit_clause_iterator = units_queue.front();
            units_queue.pop();
            int p = *(unit_clause_iterator->begin());
            model_[std::abs(p) - 1] = p < 0 ? -1 : 1;
            bool is_same_sign; // с каким знаком входит в найденный дизъюнкт
            decltype(unit_clause_iterator->begin()) literal_iterator; //итератор на элемент unordered_set<int> clause

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
                    } else if (target_clause_iterator->size() == 1) {
                        //если дизъюнкт теперь unit кладём в очередь (если дизъюнкт с такой переменной ещё не в очереди)
                        int ind = std::abs(*target_clause_iterator->begin()) - 1;

                        if (!variable_is_in_queue[ind]) {
                            units_queue.push(target_clause_iterator);
                            variable_is_in_queue[ind] = true;
                        }
                    }
                }

                //обошли все дизъюнкты содержащие p
                if (get_usage_count(p) == 0) {
                    break;
                }
                target_clause_iterator = std::find_if(next_iterator, clauses_.end(),
                                                      contains_p);
            }
        }

    }


    bool CNF::PureLiterals() { // TODO: optimize?
        bool added_unit = false;
        while (!possible_pure_queue_.empty()) {
            int p = possible_pure_queue_.front();
            possible_pure_queue_.pop();
            // пока проходило исполнение переменная могла перестать быть pure (была удалена из кнф полностью)
            if (is_pure(p)) {
                added_unit = true;
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
        return added_unit;
    }

    void CNF::AddUnitClauseFront(int p) {
        std::unordered_set<int> tmp = {p};
        clauses_.emplace_front(std::move(tmp));
        increase_usage_count(p);
        clauses_num_++;
    }

    bool CNF::IsAssigned(int p) const {
        return model_[std::abs(p) - 1] != 0;
    }

}
