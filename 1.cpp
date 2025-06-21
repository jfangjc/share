#include "word_break.h"

#include <catch2/catch.hpp>

TEST_CASE("word_break::read_lexicon works as expected") {
	auto const lexicon = word_break::read_lexicon("./english.txt");
	REQUIRE(lexicon.size() == 127142);

	SECTION("Check the first 3 words exist") {
		CHECK(lexicon.contains("aa"));
		CHECK(lexicon.contains("aah"));
		CHECK(lexicon.contains("aahed"));
	}

	SECTION("Check the last 3 words exist") {
		CHECK(lexicon.contains("zymurgy"));
		CHECK(lexicon.contains("zyzzyva"));
		CHECK(lexicon.contains("zyzzyvas"));
	}

	SECTION("Check random 6 words exist") {
		CHECK(lexicon.contains("pillions"));
		CHECK(lexicon.contains("knurls"));
		CHECK(lexicon.contains("charlatanisms"));
		CHECK(lexicon.contains("dogwood"));
		CHECK(lexicon.contains("hamates"));
		CHECK(lexicon.contains("parametrize"));
	}

	SECTION("Check random 6 non-word doesn't exist") {
		CHECK_FALSE(lexicon.contains("wefijov"));
		CHECK_FALSE(lexicon.contains("efwfmo"));
		CHECK_FALSE(lexicon.contains("32r2ufsd"));
		CHECK_FALSE(lexicon.contains("vsdndsvjk"));
		CHECK_FALSE(lexicon.contains("-fvmn="));
		CHECK_FALSE(lexicon.contains("c++"));
	}
}

TEST_CASE("at -> it") {
	auto const lexicon = std::unordered_set<std::string>{
		"at",
		"it"
	};
	auto const expected = std::vector<std::vector<std::string>>{
		{"at", "it"}
	};
	auto const ladders = word_break::word_break("atit", lexicon);

	CHECK(ladders == expected);
}

TEST_CASE("Test word_break::word_break with the expected solutions") {
	auto const lexicon = word_break::read_lexicon("./english.txt");
	REQUIRE(lexicon.size() == 127142);

	SECTION("Test dogsandragonfly") {
		auto const expected = std::vector<std::vector<std::string>>{
			{"dogs", "an", "dragonfly"}
		};
		auto const sentences = word_break::word_break("dogsandragonfly", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test effortsmock") {
		auto expected = std::vector<std::vector<std::string>>{
			{"efforts", "mock"},
			{"effort", "smock"}
		};
		auto sentences = word_break::word_break("effortsmock", lexicon);
		std::sort(sentences.begin(), sentences.end());
		std::sort(expected.begin(), expected.end());
		CHECK(sentences == expected);
	}

	SECTION("Test rutsoping") {
		auto const expected = std::vector<std::vector<std::string>>{
			{"ruts", "oping"},
		};
		auto const sentences = word_break::word_break("rutsoping", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test upcasters") {
		auto expected = std::vector<std::vector<std::string>>{
			{"upcast", "ers"},
			{"up", "casters"}
		};
		auto sentences = word_break::word_break("upcasters", lexicon);
		std::sort(sentences.begin(), sentences.end());
		std::sort(expected.begin(), expected.end());
		CHECK(sentences == expected);
	}

	SECTION("Test superman") {
		auto const expected = std::vector<std::vector<std::string>> {
			{"superman"}
		};
		auto const sentences = word_break::word_break("superman", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test thequickbrownfoxjumpsoverthelazydog") {
		auto expected = std::vector<std::vector<std::string>> {
			{"the", "quick", "brown", "fox", "jumps", "overt", "he", "lazy", "dog"},
			{"the", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"}
		};
		auto sentences = word_break::word_break("thequickbrownfoxjumpsoverthelazydog", lexicon);
		std::sort(sentences.begin(), sentences.end());
		std::sort(expected.begin(), expected.end());
		CHECK(sentences == expected);
	}

	SECTION("Test nowhere") {
		auto const expected = std::vector<std::vector<std::string>> {
			{"nowhere"}
		};
		auto const sentences = word_break::word_break("nowhere", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test meetmeatthepark") {
		auto const expected = std::vector<std::vector<std::string>> {
			{"meet", "meat", "the", "park"}
		};
		auto const sentences = word_break::word_break("meetmeatthepark", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test personor") {
		auto const expected = std::vector<std::vector<std::string>>{
			{"person", "or"}
        };
		auto const sentences = word_break::word_break("personor", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test single letter lexicon") {
		auto const expected = std::vector<std::vector<std::string>>{{ "a", "b", "c" }};
		auto const empty_lexicon = std::unordered_set<std::string>{ "a", "b", "c" };
		auto const sentences = word_break::word_break("abc", empty_lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test special character lexicon") {
		auto const expected = std::vector<std::vector<std::string>>{{ "\n" }};
		auto const empty_lexicon = std::unordered_set<std::string>{ "\n", "\t" };
		auto const sentences = word_break::word_break("\n", empty_lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test includes all possible solution") {
		auto expected = std::vector<std::vector<std::string>>{
			{ "abcd", "e" },
			{ "abc", "de" },
			{ "ab", "cde" },
			{ "a", "bcde" }
		};
		auto const empty_lexicon = std::unordered_set<std::string>{ "a", "b", "c", "d", "e",
			"ab", "cde", "abc", "de", "abcd", "bcde" };
		auto sentences = word_break::word_break("abcde", empty_lexicon);
		std::sort(sentences.begin(), sentences.end());
		std::sort(expected.begin(), expected.end());
		CHECK(sentences == expected);
	}

	SECTION("Test a long string with only one solution") {
		auto const expected = std::vector<std::vector<std::string>>{
			{ "supercalifragilisticexpialidocious" }
		};
		auto const lexicon = std::unordered_set<std::string>{ "supercalifragilisticexpialidocious", "super", "cali", "fragilistic", "expialidocious" };
		auto const sentences = word_break::word_break("supercalifragilisticexpialidocious", lexicon);
		CHECK(sentences == expected);
	}

	SECTION("Test complex overlapping words with only one solutions") {
		auto const expected = std::vector<std::vector<std::string>>{
			{ "peanut", "butter" }
		};
		auto const lexicon = std::unordered_set<std::string>{
			"pea", "nut", "butter", "peanut", "t", "peanu"
		};
		auto const sentences = word_break::word_break("peanutbutter", lexicon);
		CHECK(sentences == expected);
	}

	SECTION("Test longer string with a only one solution") {
		auto const expected = std::vector<std::vector<std::string>>{
			{ "bedbath", "andbeyond" }
		};
		auto const lexicon = std::unordered_set<std::string>{
			"bed", "bath", "and", "beyond", "bedbath", "andbeyond"
		};
		auto const sentences = word_break::word_break("bedbathandbeyond", lexicon);
		CHECK(sentences == expected);
	}

	SECTION("Test repeating pattern") {
		auto expected = std::vector<std::vector<std::string>>{
			{ "ab", "abab" },
			{ "aba", "bab" },
			{ "abab", "ab" }
		};
		auto const lexicon = std::unordered_set<std::string>{
			"a", "b", "ab", "aba", "bab", "abab"
		};
		auto sentences = word_break::word_break("ababab", lexicon);
		std::sort(sentences.begin(), sentences.end());
		std::sort(expected.begin(), expected.end());
		CHECK(sentences == expected);
	}
}

TEST_CASE("Test word_break::word_break with fail cases") {
	const std::unordered_set<std::string> lexicon = word_break::read_lexicon("./english.txt");
	REQUIRE(lexicon.size() == 127142);

	SECTION("Test vdnjweouiv") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const sentences = word_break::word_break("vdnjweouiv", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test pweiufhd") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const sentences = word_break::word_break("pweiufhd", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test rutska") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const sentences = word_break::word_break("rutsk", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test empty string") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const sentences = word_break::word_break("", lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test empty lexicon") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const empty_lexicon = std::unordered_set<std::string>{ };
		auto const sentences = word_break::word_break("dogsandragonfly", empty_lexicon);

		CHECK(sentences == expected);
	}

	SECTION("Test single letter lexicon") {
		auto const expected = std::vector<std::vector<std::string>>{ };
		auto const empty_lexicon = std::unordered_set<std::string>{ "a", "b", "c" };
		auto const sentences = word_break::word_break("defg", empty_lexicon);

		CHECK(sentences == expected);
	}
}
