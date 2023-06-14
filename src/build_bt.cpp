#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <parblo/block_tree/block_tree.hpp>
#include <parblo/block_tree/construction/sequential.hpp>

int main(int argc, const char **argv) {
    if (argc != 2) {
        std::cout << "please input file" << std::endl;
    }
    std::string       input;
    std::ifstream     t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();

    parblo::BlockTree bt(input, 2, 2, parblo::Sequential());

    std::cout << input << std::endl;
}