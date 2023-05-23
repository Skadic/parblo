#include <catch2/catch_test_macros.hpp>
#include <parblo/rolling_hash/hashed_slice.hpp>
#include <string>
#include <string_view>

const std::string TEST_STR = "Hello how are you doing?";

using parblo::HashedSlice;

TEST_CASE("hashed bytes basic ops", "[hashed bytes]") {
    const HashedSlice hashed_slice(TEST_STR, 0, TEST_STR.length(), 123456);

    SECTION("index") {
        for (size_t i = 0; i < TEST_STR.length(); i++) {
            unsigned char expected = static_cast<unsigned char>(TEST_STR.at(i));
            unsigned char actual   = hashed_slice[i];
            REQUIRE(expected == actual);
        }
    }

    SECTION("length") { REQUIRE(TEST_STR.length() == hashed_slice.length()); }

    SECTION("hash") { REQUIRE(123456 == hashed_slice.hash()); }

    SECTION("equals") {
        const HashedSlice hashed_bytes2("hello", 0, 5, 123456);
        const HashedSlice hashed_bytes3(TEST_STR, 0, TEST_STR.length(), 123);
        REQUIRE(hashed_slice == hashed_bytes2);
        REQUIRE(hashed_slice != hashed_bytes3);
    }
}

TEST_CASE("hashed bytes construction", "[hashed bytes]") {
    auto test_str_ptr = reinterpret_cast<const uint8_t *>(TEST_STR.c_str());

    SECTION("uint8_t pointer") {
        const HashedSlice hashed_slice(test_str_ptr, TEST_STR.length(), 123456);
        CHECK(test_str_ptr == hashed_slice.bytes());
        CHECK(TEST_STR.length() == hashed_slice.length());
        REQUIRE(123456 == hashed_slice.hash());
    }

    SECTION("char pointer") {
        const HashedSlice hashed_slice(TEST_STR.c_str(), TEST_STR.length(), 123456);
        CHECK(test_str_ptr == hashed_slice.bytes());
        CHECK(TEST_STR.length() == hashed_slice.length());
        REQUIRE(123456 == hashed_slice.hash());
    }

    SECTION("string") {
        const HashedSlice hashed_slice(TEST_STR, 0, TEST_STR.length(), 123456);
        CHECK(test_str_ptr == hashed_slice.bytes());
        CHECK(TEST_STR.length() == hashed_slice.length());
        REQUIRE(123456 == hashed_slice.hash());
    }

    SECTION("string view") {
        std::string_view          view(TEST_STR.c_str(), TEST_STR.length());
        const HashedSlice hashed_slice(view, 123456);
        CHECK(test_str_ptr == hashed_slice.bytes());
        CHECK(TEST_STR.length() == hashed_slice.length());
        REQUIRE(123456 == hashed_slice.hash());
    }

    SECTION("iterators") {
        const HashedSlice hashed_slice(TEST_STR.c_str(), TEST_STR.c_str() + TEST_STR.length(), 123456);
        CHECK(test_str_ptr == hashed_slice.bytes());
        CHECK(TEST_STR.length() == hashed_slice.length());
        REQUIRE(123456 == hashed_slice.hash());
    }
}