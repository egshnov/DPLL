#include "gtest/gtest.h"
#include "CNF.h"
#include "DPLL.h"
#include "errormsg.h"

TEST(CNFParsingTest, CanNotOpen) {
    solver::CNF cnf;
    EXPECT_THROW({
                     try {
                         cnf.Parse("random_filename");
                     }
                     catch (const std::string &error_message) {
                         EXPECT_EQ(errors::kClosed, error_message);
                         throw std::invalid_argument(error_message);
                     }
                 }, std::invalid_argument);
}


TEST(CNFParsingTest, NotDimacs) { //TODO: add more cases
    solver::CNF cnf;
    EXPECT_THROW({
                     try {
                         cnf.Parse("../../tests/test_input_data/Gilgamesh.cnf");
                     }
                     catch (const std::string &error_message) {
                         EXPECT_EQ(errors::kNotDIMACS, error_message);
                         throw std::invalid_argument(error_message);
                     }
                 }, std::invalid_argument);

}

TEST(CNFParsingTest, SmallCnf) {
    solver::CNF cnf;
    EXPECT_NO_THROW(cnf.Parse("../../tests/test_input_data/parsing_check_small.cnf"));
    std::string expected = "-3 2 1 0\n3 -2 0\n";
    EXPECT_EQ(cnf.ToString(), expected);
}

//TODO: fix filepath
void test_propagation(const std::string &filename, const std::string &expected) {
    solver::CNF cnf;
    EXPECT_NO_THROW(cnf.Parse("../../tests/test_input_data/" + filename));
    cnf.UnitPropagation();
    EXPECT_EQ(cnf.ToString(), expected);
}

TEST(DPLLHeuristics, UnitPropagation) {
    test_propagation("propagate1.cnf", "-2 -1 0\n3 0\n-2 1 0\n");
    test_propagation("propagate2.cnf", "1 0\n2 0\n3 0\n4 0\n");
    test_propagation("propagate3.cnf", "3 0\n4 0\n1 0\n");
    test_propagation("propagate4.cnf", "1 0\n-3 0\n2 0\n4 0\n0\n");
    test_propagation("propagate5.cnf",
                     "1 0\n4 3 0\n4 6 -2 0\n-3 -6 -2 0\n-2 -4 0\n-3 2 0\n3 6 2 0\n-4 -6 2 0\n-5 3 -6 0\n");

}

//TODO: fix filepath
void test_pure_literals(const std::string &filename, const std::string &expected) {
    solver::CNF cnf;
    EXPECT_NO_THROW(cnf.Parse("../../tests/test_input_data/" + filename));
    cnf.PureLiterals();
    EXPECT_EQ(cnf.ToString(), expected);
}

TEST(DPLLHeuristics, PureLiterals) {
    test_pure_literals("pure1.cnf", "3 0\n-1 0\n-5 4 0\n-4 5 0\n");
    test_pure_literals("pure2.cnf", "3 0\n-4 0\n-2 1 0\n2 -1 0\n");
    test_pure_literals("pure3.cnf", "2 0\n1 0\n");
    test_pure_literals("pure4.cnf", "1 0\n-2 0\n");
    test_pure_literals("pure5.cnf", "-5 0\n1 0\n4 3 0\n4 6 -2 0\n-3 6 -2 0\n-2 -4 0\n-3 2 0\n3 6 2 0\n-4 -6 2 0\n");
}

TEST(InterpreationTest, test1) {
    solver::CNF cnf;
    EXPECT_NO_THROW(cnf.Parse("../../tests/test_input_data/interpretation1.cnf"));
    EXPECT_TRUE(cnf.IsInterpretation());
}
//TODO: fix filepath
TEST(DPLL, small) {
    solver::CNF cnf;
    EXPECT_NO_THROW(cnf.Parse("../../tests/test_input_data/small.cnf"));
    solver::DPLL(cnf);
    EXPECT_TRUE(cnf.IsUnsat());
    std::cerr << cnf.ToString();
}