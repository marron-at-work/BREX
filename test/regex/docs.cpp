#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeDocsTest(const std::u8string& str) {
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

std::string fnresolve(const std::string& name, brex::NameResolverState s) {
    return name;
}

bool tryParseIntoNameMap(const std::string& name, const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return false;
    }

    if(pr.first.value()->preanchor != nullptr || pr.first.value()->postanchor != nullptr) {
        return false;
    }

    if(pr.first.value()->re->tag != brex::RegexComponentTag::Single) {
        return false;
    }

    auto sre = static_cast<const brex::RegexSingleComponent*>(pr.first.value()->re);
    if(sre->entry.isFrontCheck || sre->entry.isBackCheck || sre->entry.isNegated) {
        return false;
    }

    return nmap.insert({ name, sre->entry.opt }).second;
}

std::optional<brex::UnicodeRegexExecutor*> tryParseForNameSubTest(const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), nmap, nullptr, fnresolve, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

std::optional<brex::ASCIIRegexExecutor*> tryParseForASCIIDocsTest(const std::string& str) {
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

#define ACCEPTS_TEST_UNICODE_DOCS(RE, STR, ACCEPT) {auto uustr = brex::UnicodeString(STR); brex::ExecutorError err; auto accepts = executor->test(&uustr, err); BOOST_CHECK(err == brex::ExecutorError::Ok); BOOST_CHECK(accepts == ACCEPT); }

#define ACCEPTS_TEST_ASCII_DOCS(RE, STR, ACCEPT) {auto uustr = brex::ASCIIString(STR); brex::ExecutorError err; auto accepts = executor->test(&uustr, err); BOOST_CHECK(err == brex::ExecutorError::Ok); BOOST_CHECK(accepts == ACCEPT); }

BOOST_AUTO_TEST_SUITE(Docs)

BOOST_AUTO_TEST_SUITE(Readme)
BOOST_AUTO_TEST_CASE(thisisaliteral) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"this is a literal\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"this is a literal", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"this is a literalthis is a literal", true);
    
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abcd", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"this is ", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8" this is a literal", false);
}
BOOST_AUTO_TEST_CASE(thisisaliteralpepper) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"unicode literal 🌶\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"unicode literal 🌶", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abcd", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"unicode ", false);
}
BOOST_AUTO_TEST_CASE(thisisaliteralascii) {
    auto texecutor = tryParseForASCIIDocsTest("/'ascii literals %x59;'/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_ASCII_DOCS(executor, "bob", false);
    ACCEPTS_TEST_ASCII_DOCS(executor, "ascii literals Y", true);
    ACCEPTS_TEST_ASCII_DOCS(executor, "ascii literals Z", false);
}
BOOST_AUTO_TEST_CASE(twoescapesparse) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"%x7;%x0;\"/");
    BOOST_CHECK(texecutor.has_value());

    auto uexecutor = tryParseForUnicodeDocsTest(u8"/\"%a;%NUL;\"/");
    BOOST_CHECK(uexecutor.has_value());
}

BOOST_AUTO_TEST_CASE(nameddigit) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/[+-]${Digit}+/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abc", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"0", false);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"+2", true);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
