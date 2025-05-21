#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <memory> // Required for std::unique_ptr

#include "../../src/regex/brex.h" // Assuming brex.h includes common.h or json.hpp
#include "../../src/common.h"    // For json typedef and potentially other utilities

// Helper function to perform the round-trip test
// 1. Parse initial JSON string to main C++ object (obj1)
// 2. Emit JSON from obj1 (json_emitted1)
// 3. (Optional) Compare json_emitted1 with a canonical/expected JSON string if provided
// 4. Parse json_emitted1 to another C++ object (obj2)
// 5. Emit JSON from obj2 (json_emitted2)
// 6. Compare json_emitted1 and json_emitted2
void perform_round_trip_test(const std::string& initial_json_str, const std::string* expected_json_str_after_first_emit = nullptr) {
    // 1. Parse initial JSON string to main C++ object (obj1)
    nlohmann::json initial_json_obj = nlohmann::json::parse(initial_json_str);
    std::unique_ptr<brex::Regex> regex_obj1(brex::Regex::jparse(initial_json_obj));
    BOOST_REQUIRE(regex_obj1 != nullptr); // Ensure parsing was successful

    // 2. Emit JSON from obj1 (json_emitted1)
    nlohmann::json json_emitted1;
    regex_obj1->emitJSON(json_emitted1);

    // 3. (Optional) Compare json_emitted1 with a canonical/expected JSON string
    if (expected_json_str_after_first_emit != nullptr) {
        nlohmann::json expected_json_obj = nlohmann::json::parse(*expected_json_str_after_first_emit);
        BOOST_CHECK_EQUAL(json_emitted1.dump(4), expected_json_obj.dump(4));
    } else {
        // If no specific expected output for the first emit, compare it with the initial input.
        // This is for cases where emitJSON output should be identical to jparse input.
        BOOST_CHECK_EQUAL(json_emitted1.dump(4), initial_json_obj.dump(4));
    }

    // 4. Parse json_emitted1 to another C++ object (obj2)
    std::unique_ptr<brex::Regex> regex_obj2(brex::Regex::jparse(json_emitted1));
    BOOST_REQUIRE(regex_obj2 != nullptr); // Ensure parsing of emitted JSON was successful

    // 5. Emit JSON from obj2 (json_emitted2)
    nlohmann::json json_emitted2;
    regex_obj2->emitJSON(json_emitted2);

    // 6. Compare json_emitted1 and json_emitted2
    // This is the core round-trip check: emit(parse(emit(parse(X)))) == emit(parse(X))
    BOOST_CHECK_EQUAL(json_emitted1.dump(4), json_emitted2.dump(4));
}


BOOST_AUTO_TEST_SUITE(EmitJSONTestSuite)

BOOST_AUTO_TEST_CASE(TestLiteralOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "LiteralOpt",
            "charcodes": [97, 98, 99],
            "isunicode": true
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [97, 98, 99], "isunicode": true}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestCharRangeOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "CharRangeOpt",
            "compliment": false,
            "isunicode": true,
            "range": [{"lb": 97, "ub": 122}]
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "CharRangeOpt", "compliment": false, "isunicode": true,
                "range": [{"lb": 97, "ub": 122}]
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestCharClassDotOpt) {
    std::string initial_json_str = R"({
        "re": {"tag": "CharClassDotOpt"}
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "CharClassDotOpt"}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestNamedRegexOpt) {
    std::string initial_json_str = R"({
        "re": {"tag": "NamedRegexOpt", "rname": "MyRegex"}
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "NamedRegexOpt", "rname": "MyRegex"}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestEnvRegexOpt) {
    std::string initial_json_str = R"({
        "re": {"tag": "EnvRegexOpt", "ename": "MY_ENV_VAR"}
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "EnvRegexOpt", "ename": "MY_ENV_VAR"}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestStarRepeatOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "StarRepeatOpt",
            "repeat": {"tag": "LiteralOpt", "charcodes": [120], "isunicode": true}
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "StarRepeatOpt",
                "repeat": {"tag": "LiteralOpt", "charcodes": [120], "isunicode": true}
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestPlusRepeatOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "PlusRepeatOpt",
            "repeat": {"tag": "LiteralOpt", "charcodes": [121], "isunicode": false}
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "PlusRepeatOpt",
                "repeat": {"tag": "LiteralOpt", "charcodes": [121], "isunicode": false}
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestRangeRepeatOptBounded) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "RangeRepeatOpt",
            "low": 2,
            "high": 5,
            "repeat": {"tag": "CharClassDotOpt"}
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "RangeRepeatOpt", "low": 2, "high": 5,
                "repeat": {"tag": "CharClassDotOpt"}
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestRangeRepeatOptUnbounded) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "RangeRepeatOpt",
            "low": 3,
            "repeat": {"tag": "CharClassDotOpt"}
        }
    })";
    std::string expected_json_str_after_first_emit = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "RangeRepeatOpt", "low": 3, "high": null,
                "repeat": {"tag": "CharClassDotOpt"}
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str_after_first_emit);
}


BOOST_AUTO_TEST_CASE(TestOptionalOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "OptionalOpt",
            "opt": {"tag": "LiteralOpt", "charcodes": [63], "isunicode": true}
        }
    })";
     std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                 "tag": "OptionalOpt",
                 "opt": {"tag": "LiteralOpt", "charcodes": [63], "isunicode": true}
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestAnyOfOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "AnyOfOpt",
            "opts": [
                {"tag": "LiteralOpt", "charcodes": [97], "isunicode": true},
                {"tag": "LiteralOpt", "charcodes": [98], "isunicode": true}
            ]
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "AnyOfOpt",
                "opts": [
                    {"tag": "LiteralOpt", "charcodes": [97], "isunicode": true},
                    {"tag": "LiteralOpt", "charcodes": [98], "isunicode": true}
                ]
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestSequenceOpt) {
    std::string initial_json_str = R"({
        "re": {
            "tag": "SequenceOpt",
            "regexs": [
                {"tag": "LiteralOpt", "charcodes": [102, 111, 111], "isunicode": true},
                {"tag": "LiteralOpt", "charcodes": [98, 97, 114], "isunicode": false}
            ]
        }
    })";
    std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {
                "tag": "SequenceOpt",
                "regexs": [
                    {"tag": "LiteralOpt", "charcodes": [102, 111, 111], "isunicode": true},
                    {"tag": "LiteralOpt", "charcodes": [98, 97, 114], "isunicode": false}
                ]
            }
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}

BOOST_AUTO_TEST_CASE(TestRegexSingleComponent) {
    // This test's initial JSON already matches the emitted structure for "re"
    // because it explicitly includes "isNegated", "isFrontCheck", "isBackCheck", and "opt".
    std::string initial_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false,
            "isFrontCheck": false,
            "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [120, 121, 122], "isunicode": true}
        }
    })";
    // No separate expected string needed as initial_json_str should match json_emitted1
    perform_round_trip_test(initial_json_str); 
}

BOOST_AUTO_TEST_CASE(TestRegexAllOfComponent) {
    std::string initial_json_str = R"({
        "re": [
            {
                "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
                "opt": {"tag": "LiteralOpt", "charcodes": [97], "isunicode": true}
            },
            {
                "isNegated": true, "isFrontCheck": false, "isBackCheck": false,
                "opt": {"tag": "LiteralOpt", "charcodes": [98], "isunicode": true}
            }
        ]
    })";

    std::string expected_json_str_after_first_emit = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": [
            {
                "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
                "opt": {"tag": "LiteralOpt", "charcodes": [97], "isunicode": true}
            },
            {
                "isNegated": true, "isFrontCheck": false, "isBackCheck": false,
                "opt": {"tag": "LiteralOpt", "charcodes": [98], "isunicode": true}
            }
        ]
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str_after_first_emit);
}

BOOST_AUTO_TEST_CASE(TestFullRegexWithAnchorsAndFlags) {
    // This test's initial JSON for "re" is direct RegexOpt, 
    // but "preanchor" and "postanchor" are full ToplevelEntry structures.
    std::string initial_json_str = R"({
        "isPath": true,
        "isChar": true,
        "preanchor": { // Already in emitted format
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [94], "isunicode": true}
        },
        "re": { // Direct RegexOpt
            "tag": "LiteralOpt", "charcodes": [109, 97, 105, 110], "isunicode": true
        },
        "postanchor": { // Already in emitted format
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [36], "isunicode": true}
        }
    })";
    std::string expected_json_str = R"({
        "isPath": true, "isChar": true,
        "preanchor": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [94], "isunicode": true}
        },
        "re": { // Expected emitted format for "re"
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [109, 97, 105, 110], "isunicode": true}
        },
        "postanchor": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [36], "isunicode": true}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}


BOOST_AUTO_TEST_CASE(TestRegexFlags_isPath_false_isChar_false) {
    // This test's initial JSON already includes top-level flags and "re" is direct RegexOpt.
    std::string initial_json_str = R"({
        "isPath": false,
        "isChar": false, 
        "re": {"tag": "LiteralOpt", "charcodes": [97, 98], "isunicode": false}
    })";
     std::string expected_json_str = R"({
        "isPath": false, "isChar": false, "preanchor": null, "postanchor": null,
        "re": {
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "LiteralOpt", "charcodes": [97, 98], "isunicode": false}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str);
}


BOOST_AUTO_TEST_CASE(TestRegex_NullAnchors) {
    std::string initial_json_str = R"({
        "preanchor": null,
        "postanchor": null,
        "re": {"tag": "CharClassDotOpt"}
    })";
    
    std::string expected_json_str_after_first_emit = R"({
        "isPath": false, "isChar": false, 
        "preanchor": null, "postanchor": null,
        "re": { // Expected emitted format for "re"
            "isNegated": false, "isFrontCheck": false, "isBackCheck": false,
            "opt": {"tag": "CharClassDotOpt"}
        }
    })";
    perform_round_trip_test(initial_json_str, &expected_json_str_after_first_emit);
}


BOOST_AUTO_TEST_SUITE_END()
