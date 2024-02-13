#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeTest(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::RegexOpt*> emptymap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), emptymap, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}


std::optional<brex::ASCIIRegexExecutor*> tryParseForASCIITest(const std::string& str) {
    auto pr = brex::RegexParser::parseASCIIRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::RegexOpt*> emptymap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileASCIIRegexToExecutor(pr.first.value(), emptymap, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

#define ACCEPTS_TEST_UNICODE(RE, STR, ACCEPT) {auto uustr = brex::UnicodeString(STR); auto accepts = executor->test(&uustr); BOOST_CHECK(accepts == ACCEPT); }

#define ACCEPTS_TEST_ASCII(RE, STR, ACCEPT) {auto uustr = brex::ASCIIString(STR); auto accepts = executor->test(&uustr); BOOST_CHECK(accepts == ACCEPT); }

BOOST_AUTO_TEST_SUITE(Test)

////
//Range
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(abc) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"abc\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"abc", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);

    ACCEPTS_TEST_UNICODE(executor, u8"abcd", false);
    ACCEPTS_TEST_UNICODE(executor, u8"xab", false);
}

BOOST_AUTO_TEST_CASE(eps) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"abc", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}

BOOST_AUTO_TEST_CASE(literal) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a🌵c\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a🌵c", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", false);
}

BOOST_AUTO_TEST_CASE(escape) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"%%;%underscore;%x32;\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"%_2", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"%_aa", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(abc) {
    auto texecutor = tryParseForASCIITest("/'abc'/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_ASCII(executor, "abc", true);
    ACCEPTS_TEST_ASCII(executor, "ab", false);
    ACCEPTS_TEST_ASCII(executor, "", false);

    ACCEPTS_TEST_ASCII(executor, "abcd", false);
    ACCEPTS_TEST_ASCII(executor, "xab", false);
}

BOOST_AUTO_TEST_CASE(eps) {
    auto texecutor = tryParseForASCIITest("/''/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_ASCII(executor, "abc", false);
    ACCEPTS_TEST_ASCII(executor, "", true);
}

BOOST_AUTO_TEST_CASE(escape) {
    auto texecutor = tryParseForASCIITest("/'%%;%underscore;%x32;'/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_ASCII(executor, "%_2", true);
    ACCEPTS_TEST_ASCII(executor, "aaa", false);
    ACCEPTS_TEST_ASCII(executor, "%_aa", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(opts3) {
    auto texecutor = tryParseForUnicodeTest(u8"/[06a]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"6", true);
    ACCEPTS_TEST_UNICODE(executor, u8"1", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(optsrng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"3", true);
    ACCEPTS_TEST_UNICODE(executor, u8"9", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(optshat) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0^]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"^", true);
    ACCEPTS_TEST_UNICODE(executor, u8"1", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(combos) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9 +]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"+", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
}
BOOST_AUTO_TEST_CASE(compliment) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^A-Z]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"A", false);
    ACCEPTS_TEST_UNICODE(executor, u8"Q", false);
}
BOOST_AUTO_TEST_CASE(complimet2) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^A-Z0a-c]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"^", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", true);
    ACCEPTS_TEST_UNICODE(executor, u8"0", false);
    ACCEPTS_TEST_UNICODE(executor, u8"b", false);
}
BOOST_AUTO_TEST_CASE(emoji) {
    auto texecutor = tryParseForUnicodeTest(u8"/[🌵-🌶]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"🌽", false);
}
BOOST_AUTO_TEST_CASE(complimentemoji) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^🌵-🌶]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌽", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", false);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
/*
xxxx;
*/
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Dot
BOOST_AUTO_TEST_SUITE(Dot)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(simple) {
    auto texecutor = tryParseForUnicodeTest(u8"/./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8".", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", true);

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(dotrng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[.b]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8".", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", false);

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(combobe) {
    auto texecutor = tryParseForUnicodeTest(u8"/.\"b\"./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8".b.", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbx", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
}
BOOST_AUTO_TEST_CASE(comborng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"9b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"4🌶", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
/*
xxxx;
*/
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
