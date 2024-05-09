#include "gtest/gtest.h"
#include "CNF.h"
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

TEST(CNFUnitPropagation, )

