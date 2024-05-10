#pragma once

#include <unordered_set>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>

/* как хранится и что делается: */


/* Интерпретация:
 * набор unit clause (дизъюнкты из одной переменной) СОДЕРЖАЩИЙ ВСЕ ПЕРЕМЕННЫЕ т.е в конце формула превратится в интерпретацию
 */

/*Алгоритм: */

/* 0.1 Если содержит пустой дизъюнкт значит UNSAT*/

/* 0.2 Если все дизъюнкты - unit clause и входят все переменные - получили интерпретацию */

/* 1. Unit propagation
 * пока есть unit:
 * Идём по дизъюнктам спрашиваем -- нынешний unit или нет
 * если дизъюнкт -- unit заходим в нужную переменную
 * из переменной обходим все дизъюнкты её содержащие (кроме нынешнего):
 * если входит с тем же знаком -- удаляем дизъюнкт
 * если входит с другим знаком -- удаляем её из дизъюнкта -- вроде на этом моменте уже можно понять что unsat если теперь пусто и выйти с ветки
 */

/* 2. Pure literal
 * Идем по переменным спрашиваем -- нынешняя pure или нет
 * если pure обходим все дизъюнкты содержащие данную переменную и удаляем их
 * добавляем unit clause с этой переменной
 */

/* 3. choose and set variable to assign
 * Добавляем в список дизъюнктов unit clause со знаком нашей переменной
 * сначала с плюсом потом ,если не вышло, с минусом
 * */


namespace solver {
    class CNF {
    private:
        using Clause = std::unordered_set<int>;
    public:
        std::list<Clause> clauses_;
    private:
        struct sign_counter {
            int plus_ = 0; // число вхождений в кнф с +
            int minus_ = 0; // число вхождений в кнф с -
        };

        //счетчик числа использований для pure literals
        std::vector<sign_counter> variable_sign_usage_count_;
        //std::unordered_map
        //переменная которая на данной ветке была обработана как pure
        // уже не будет использована как pure повторно (на данной ветке)
        //TODO: rm public
        std::queue<int> possible_pure_queue;
        //очередь т.к. после инициализации в очереди лежат переменные которые точно pure
        int variables_num_;
        int clauses_num_;
        bool contains_empty_ = false;

        void increase_usage_count(int p);

        void decrease_usage_count_and_check_if_pure(int p);

        int get_usage_count(int p);

        bool is_pure(int p);

        void Init(std::ifstream &stream);

    public:
        CNF() = default;

        [[nodiscard]] bool IsUnsat() const {
            return contains_empty_;
        }

        [[nodiscard]] int GetVariablesNum() const {
            return variables_num_;
        }

        void AddUnitClauseFront(int p);

        std::string ToString();

        void Parse(const std::string &filename);

        void UnitPropagation();

        bool IsInterpretation();

        void PureLiterals();
    };
}
