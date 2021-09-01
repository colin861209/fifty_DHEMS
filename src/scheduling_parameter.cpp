#include <iostream>
#include "scheduling_parameter.hpp"
std::string row_num_string;

void saving_coefAndBnds_rowNum(int coef_row_num, int coef_diff, int bnd_row_num, int bnd_diff) {

    row_num_string += "\t@:    "+ std::to_string(coef_row_num-coef_diff)+ " ~ " +std::to_string( coef_row_num - 1)
    +"\t "+ std::to_string(bnd_row_num-bnd_diff) + " ~ " + std::to_string(bnd_row_num - 1) +"\n";

}

void display_coefAndBnds_rowNum() {

    std::cout << row_num_string << std::endl;
}