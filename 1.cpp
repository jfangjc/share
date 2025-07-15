#include "./filtered_string_view.h"

#include <catch2/catch.hpp>
#include <cstring>
#include <set>

TEST_CASE("Test Default Constructor") {
    auto sv = fsv::filtered_string_view();
    CHECK(sv.data() == nullptr);
    CHECK(sv.size() == 0);
}

TEST_CASE("Test Implicit String Constructor") {
    auto s = std::string("cat");
    auto sv = fsv::filtered_string_view(s);
    CHECK(!std::strcmp(sv.data(), "cat"));
    CHECK(sv.size() == 3);
    CHECK(sv.predicate()('1') == true);
}

TEST_CASE("Test String Constructor with Predicate") {
    auto s = std::string("cat");
    auto pred = [](const char& c) { return c == 'a'; };
    auto sv = fsv::filtered_string_view(s, pred);
    CHECK(!std::strcmp(sv.data(), "cat"));
    CHECK(sv.size() == 1);
    CHECK(sv.predicate()('c') == false);
    CHECK(sv.predicate()('a') == true);
}

TEST_CASE("Test Implicit Null-Terminated String Constructor") {
    auto sv = fsv::filtered_string_view("cat");
    CHECK(!std::strcmp(sv.data(), "cat"));
    CHECK(sv.size() == 3);
    CHECK(sv.predicate()('1') == true);
}

TEST_CASE("Test Null-Terminated String with Predicate Constructor") {
    auto pred = [](const char& c) { return c == 'a'; };
    auto sv = fsv::filtered_string_view("cat", pred);
    CHECK(!std::strcmp(sv.data(), "cat"));
    CHECK(sv.size() == 1);
    CHECK(sv.predicate()('c') == false);
    CHECK(sv.predicate()('a') == true);
}

TEST_CASE("Test Copy and Move constructor") {
    auto sv1 = fsv::filtered_string_view("bulldog");
    const auto copy = sv1;

    CHECK(copy.data() == sv1.data());
    CHECK(copy.size() == sv1.size());
    CHECK(copy.predicate()('1') == sv1.predicate()('1'));

    const auto move = std::move(sv1);
    CHECK(!std::strcmp(move.data(), "bulldog"));
    CHECK(move.size() == 7);
    CHECK(sv1.data() == nullptr);
    CHECK(sv1.size() == 0);
}

TEST_CASE("Test Copy Assignment") {
    auto pred = [](const char& c) { return c == '4' || c == '2'; };
    auto fsv1 = fsv::filtered_string_view("42 bro", pred);
    auto fsv2 = fsv::filtered_string_view();
    fsv2 = fsv1;
    CHECK(fsv1 == fsv2);
}

TEST_CASE("Test Move Assignment") {
    auto pred = [](const char& c) { return c == '8' || c == '9'; };
    auto fsv1 = fsv::filtered_string_view("'89 baby", pred);
    auto fsv2 = fsv::filtered_string_view();
    fsv2 = std::move(fsv1);

    CHECK(!std::strcmp(fsv2.data(), "'89 baby"));
    CHECK(fsv2 == "89");
    CHECK(fsv2.size() == 2);
    assert(fsv1.data() == nullptr);
    CHECK(fsv1.size() == 0);
}

TEST_CASE("Test Subscript") {
    auto pred = [](const char& c) { return c == '9' || c == '0' || c == ' '; };
    auto fsv1 = fsv::filtered_string_view("only 90s kids understand", pred);
    CHECK(fsv1[0] == ' ');
    CHECK(fsv1[1] == '9');
    CHECK(fsv1[2] == '0');
    CHECK(fsv1[3] == ' ');
}

TEST_CASE("Test at") {
    SECTION("success") {
        auto vowels = std::set<char>{'a', 'A', 'e', 'E', 'i', 'I', 'o', 'O', 'u', 'U'};
        auto is_vowel = [&vowels](const char& c) { return vowels.contains(c); };
        auto sv = fsv::filtered_string_view("Malamute", is_vowel);
        CHECK(sv.at(0) == 'a');
    }

    SECTION("fail") {
        auto sv = fsv::filtered_string_view("");
        CHECK_THROWS_AS(sv.at(0), std::domain_error);

        auto pred = [](const char& c) { return c == '9' || c == '0'; };
        sv = fsv::filtered_string_view("only 90s kids understand", pred);
        CHECK(sv.at(0) == '9');
        CHECK(sv.at(1) == '0');
        CHECK_THROWS_AS(sv.at(2), std::domain_error);
    }
}

TEST_CASE("Test size") {
    auto sv = fsv::filtered_string_view("Maltese");
    CHECK(sv.size() == 7);

    sv = fsv::filtered_string_view("Toy Poodle", [](const char& c) { return c == 'o'; });
    CHECK(sv.size() == 3);
}

TEST_CASE("Test empty") {
    auto sv = fsv::filtered_string_view("Australian Shephard");
    auto empty_sv = fsv::filtered_string_view();
    CHECK(sv.empty() == false);
    CHECK(empty_sv.empty() == true);

    sv = fsv::filtered_string_view("Border Collie", [](const char& c) { return c == 'z'; });
    CHECK(sv.empty() == true);
}

TEST_CASE("Test data") {
    auto s = "Sum 42";
    auto sv = fsv::filtered_string_view(s, [](const char&) { return false; });
    auto result = std::string();
    for (auto ptr = sv.data(); *ptr; ++ptr) {
        result.push_back(*ptr);
    }
    CHECK(result == "Sum 42");

    const char* s1 = "abc";
    sv = fsv::filtered_string_view(s1);
    CHECK(sv.data() == s1);
}

TEST_CASE("Test predicate") {
    const auto print_and_return_true = [](const char& c) { return c == 'h'; };
    const auto s = fsv::filtered_string_view("doggo", print_and_return_true);

    const auto& predicate = s.predicate();
    CHECK(predicate('h') == true);
    CHECK(predicate('i') == false);
}

TEST_CASE("Test Equality Comparison") {
    auto const lo = fsv::filtered_string_view("aaa");
    auto const hi = fsv::filtered_string_view("zzz");

    CHECK_FALSE(lo == hi);
    CHECK(lo != hi);
}

TEST_CASE("Test Relational Comparison") {
    auto lo = fsv::filtered_string_view("horn");
    auto hi = fsv::filtered_string_view("a mule", [](char const& c) { return c != 'a' && c != ' '; });

    CHECK(lo < hi);
    CHECK(lo <= hi);
    CHECK_FALSE(lo > hi);
    CHECK_FALSE(lo >= hi);

    lo = fsv::filtered_string_view("horn");
    hi = fsv::filtered_string_view("horn");
    CHECK_FALSE(lo < hi);
    CHECK(lo <= hi);
    CHECK_FALSE(lo > hi);
    CHECK(lo >= hi);

    lo = fsv::filtered_string_view("abc");
    hi = fsv::filtered_string_view("bcd");
    CHECK(lo < hi);
    CHECK(lo <= hi);
    CHECK_FALSE(lo > hi);
    CHECK_FALSE(lo >= hi);

    lo = fsv::filtered_string_view("abc");
    hi = fsv::filtered_string_view("abcd");
    CHECK(lo < hi);
    CHECK(lo <= hi);
    CHECK_FALSE(lo > hi);
    CHECK_FALSE(lo >= hi);
}

TEST_CASE("Test Output Stream") {
    auto fsv = fsv::filtered_string_view("c++ > rust > java", [](const char& c) { return c == 'c' || c == '+'; });
    CHECK((std::ostringstream() << fsv).str() == "c++");
}

TEST_CASE("Test Compose") {
    auto best_languages = fsv::filtered_string_view("c / c++");
    auto vf = std::vector<fsv::filter>{[](const char& c) { return c == 'c' || c == '+' || c == '/'; },
                                       [](const char& c) { return c > ' '; },
                                       [](const char&) { return true; }};
    auto sv = fsv::compose(best_languages, vf);
    CHECK(sv == "c/c++");

    best_languages = fsv::filtered_string_view("abcdefgh");
    vf = std::vector<fsv::filter>{[](const char& c) { return c < 'z'; },
                                  [](const char& c) { return c < 'h'; },
                                  [](const char& c) { return c < 'd'; }};
    sv = fsv::compose(best_languages, vf);
    CHECK(sv == "abc");

    best_languages = fsv::filtered_string_view("abcdefgh");
    vf = std::vector<fsv::filter>{[](const char& c) { return c < 'a'; },
                                  [](const char& c) { return c < 'h'; },
                                  [](const char& c) { return c < 'd'; }};
    sv = fsv::compose(best_languages, vf);
    CHECK(sv == "");
}

TEST_CASE("Test Split") {
    auto wentworth = fsv::filtered_string_view("Malcom? Bligh? Turnbull", [](const char& c) { return c != '?'; });
    auto token = fsv::filtered_string_view(" 2015", [](const char& c) { return c == ' '; });
    auto representative = fsv::split(wentworth, token);

    CHECK(std::string(wentworth) == "Malcom Bligh Turnbull");
    CHECK(std::string(token) == " ");
    CHECK(std::string(representative.at(0)) == "Malcom");
    CHECK(std::string(representative.at(1)) == "Bligh");
    CHECK(std::string(representative.at(2)) == "Turnbull");

    auto fsv = fsv::filtered_string_view("fishing");
    auto empty_tok = fsv::filtered_string_view("");
    auto empty_split = fsv::split(fsv, empty_tok);
    CHECK(empty_split.at(0) == "fishing");

    auto empty_fsv = fsv::filtered_string_view("");
    empty_split = fsv::split(empty_fsv, token);
    CHECK(empty_split.at(0) == "");

    auto fill_tok = fsv::filtered_string_view("robert");
    auto fill_split = fsv::split(fsv, fill_tok);
    CHECK(fill_split.at(0) == "fishing");

    auto sv = fsv::filtered_string_view("xax");
    auto tok = fsv::filtered_string_view("x");
    auto v = fsv::split(sv, tok);
    auto expected = std::vector<fsv::filtered_string_view>{"", "a", ""};
    CHECK(v == std::vector<fsv::filtered_string_view>{"", "a", ""});

    sv = fsv::filtered_string_view("xx");
    tok = fsv::filtered_string_view("x");
    v = fsv::split(sv, tok);
    expected = std::vector<fsv::filtered_string_view>{"", "", ""};
    CHECK(v == expected);
}

TEST_CASE("Test Substr") {
    auto sv = fsv::filtered_string_view("new york city");

    CHECK(fsv::substr(sv, 4, 4) == "york");

    sv = fsv::filtered_string_view("black pen");
    CHECK(fsv::substr(sv, 6) == "pen");

    sv = fsv::filtered_string_view("the right honourable. anthony charles lynton BLAIR",
                                   [](const char& c) { return c >= 'A' and c <= 'Z'; });
    CHECK_THROWS_AS(fsv::substr(sv, 6), std::out_of_range);

    sv = fsv::filtered_string_view("notebook");
    CHECK(fsv::substr(sv, 8).empty() == true);
    CHECK(fsv::substr(sv, 3, 0).empty() == true);
}

TEST_CASE("Test Iterator") {
    auto fsv = fsv::filtered_string_view("ned");
    auto iter = fsv.begin();

    CHECK((std::ostringstream() << *iter).str() == "n");
    ++iter;
    CHECK((std::ostringstream() << *iter).str() == "e");
    ++iter;
    CHECK((std::ostringstream() << *iter).str() == "d");
    ++iter;
    CHECK(iter == fsv.end());

    fsv = fsv::filtered_string_view("samoyed", [](const char& c) {
        return !(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
    });
    iter = fsv.begin();
    CHECK((std::ostringstream() << *iter).str() == "s");
    CHECK((std::ostringstream() << *std::next(iter)).str() == "m");
    CHECK((std::ostringstream() << *std::next(iter, 2)).str() == "y");
    CHECK((std::ostringstream() << *std::next(iter, 3)).str() == "d");

    const auto str = std::string("tosa");
    const auto s = fsv::filtered_string_view(str);
    auto it = s.cend();
    CHECK((std::ostringstream() << *std::prev(it)).str() == "a");
    CHECK((std::ostringstream() << *std::prev(it, 2)).str() == "s");
}

TEST_CASE("Test Range") {
    auto s = fsv::filtered_string_view("puppy", [](const char& c) { return !(c == 'u' || c == 'y'); });
    auto v = std::vector<char>{s.begin(), s.end()};
    CHECK((std::ostringstream() << v[0]).str() == "p");
    CHECK((std::ostringstream() << v[1]).str() == "p");
    CHECK((std::ostringstream() << v[2]).str() == "p");

    s = fsv::filtered_string_view("milo", [](const char& c) { return !(c == 'i' || c == 'o'); });
    v = std::vector<char>{s.rbegin(), s.rend()};
    CHECK((std::ostringstream() << v[0]).str() == "l");
    CHECK((std::ostringstream() << v[1]).str() == "m");
}
