#pragma once

#include <unordered_set>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <list>

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
        std::list<Clause> clauses_;
        // мини эвристика? - unitы пихаем в начало
        //TODO: проверить на дз с замерами дает ли хоть какой-то прирост

        struct sign_counter {
            int plus_ = 0; // число вхождений в кнф с +
            int minus_ = 0; // число вхождений в кнф с -
        };

        std::vector<sign_counter> variable_sign_usage_count_;
        // хотим что бы std::find_if(clauses, is_unit_clause) скипал куски интерпретации
        //кусок интерпретации - unit_clause чья переменная не входит в другие дизъюнкты

        int variables_num_;
        int clauses_num_;
        bool contains_empty_ = false;

        void increase_usage_count(int p);

        void decrease_usage_count(int p);

        int get_usage_count(int p);

        int find_pure();

    public:
        CNF() = default;

        bool IsUnsat() {
            return clauses_.empty();
        }

        [[nodiscard]] bool ContainsEmpty() const {
            return contains_empty_;
        }

        void AddClauseFront(int p);

        std::string CnfToString();

        void Init(std::ifstream &stream);

        void UnitPropagation();

        bool IsInterpetation();

        void PureLiterals();
    };
}
