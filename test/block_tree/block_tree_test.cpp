#include <parblo/block_tree/block_tree.hpp>
#include <parblo/block_tree/construction/sequential.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("soos", "[block tree][sequential]") {
    const std::string s = "hallohallohallohallo";

    parblo::BlockTree bt(s, 4, 5, parblo::Sequential());
}
