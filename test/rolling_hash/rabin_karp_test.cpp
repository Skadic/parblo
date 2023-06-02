#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <parblo/rolling_hash/rabin_karp.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <parblo/rolling_hash/hashed_slice.hpp>
#include <sstream>
#include <string_view>
#include <unordered_set>

constexpr std::string RESOURCE_PATH = "./resources/";

using namespace parblo;

std::string read_res_file(std::string path) {
    std::ifstream     ifs(std::filesystem::canonical(RESOURCE_PATH + path));
    std::stringstream buf;
    buf << ifs.rdbuf();
    return buf.str();
}

/// Ensure the constructurs yield the same hasher
TEST_CASE("rabin karp construction", "[rabin karp]") {
    const std::string s           = "hellohello";
    const size_t      len         = s.length();
    const size_t      window_size = 5;

    const RabinKarp rk(reinterpret_cast<const uint8_t *>(s.c_str()), len, window_size);

    SECTION("char pointer") {
        const RabinKarp rk2 = RabinKarp(s.c_str(), len, window_size);
        REQUIRE(rk == rk2);
    }

    SECTION("string") {
        const RabinKarp rk2 = RabinKarp(s, 0, window_size);
        REQUIRE(rk == rk2);
    }
} 

/// Rudimentally ensure that the standard hash algorithm
/// and the rabin karp hasher produce the same number of distinct hashes
TEST_CASE("rabin karp large", "[rabin karp]") {
    const std::string file         = GENERATE("as.txt", "dna.txt", "einstein.txt");
    const std::string file_content = read_res_file(file);
    REQUIRE_FALSE(file_content.empty());
    const size_t len         = file_content.length();
    const size_t window_size = 32;

    std::unordered_set<std::string_view> normal_hash_map;
    std::unordered_set<HashedSlice>      rk_hash_map;
    RabinKarp                            rk(file_content, 0, window_size);
    for (size_t i = 0; i <= file_content.length() - window_size; i++) {
        std::string_view view(file_content.begin() + i, file_content.begin() + i + window_size);
        normal_hash_map.insert(view);
        rk_hash_map.insert(rk.hashed_slice());
        rk.advance();
    }
    REQUIRE(normal_hash_map.size() == rk_hash_map.size());
}

/// Ensure the hasher yields the same values for equal substrings,
/// and different hashes for different substrings
TEST_CASE("rabin karp equal substrings", "[rabin karp]") {
    const std::string s           = "hellohello";
    const size_t      len         = s.length();
    const size_t      window_size = 5;

    RabinKarp         rk(s, 0, window_size);
    const HashedSlice first_hash = rk.hashed_slice();
    for (size_t i = 0; i < 4; i++) {
        rk.advance();
        REQUIRE(first_hash != rk.hashed_slice());
    }
    rk.advance();
    const HashedSlice last_hash = rk.hashed_slice();
    REQUIRE(first_hash == last_hash);
}