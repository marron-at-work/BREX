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
#define ACCEPTS_TEST_UNICODE_RNG_DOCS(RE, STR, SPOS, EPOS, ACCEPT) {auto uustr = brex::UnicodeString(STR); brex::ExecutorError err; auto accepts = executor->test(&uustr, SPOS, EPOS, true, true, err); BOOST_CHECK(err == brex::ExecutorError::Ok); BOOST_CHECK(accepts == ACCEPT); }

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
BOOST_AUTO_TEST_CASE(notsuffix) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/!(\".txt\" | \".pdf\")/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abc", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8".txt", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8".pdf", false);
}
BOOST_AUTO_TEST_CASE(kyversion1) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/[0-9]{5}(\"-\"[0-9]{3})? & ^\"4\"[0-2]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"87111", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"41502", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"49502", false);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abc", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"123", false);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502-123", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502-abc", false);
}
BOOST_AUTO_TEST_CASE(kyversion2) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("Zipcode", u8"/[0-9]{5}(\"-\"[0-9]{3})?/", nmap));
    BOOST_CHECK(tryParseIntoNameMap("PrefixKY", u8"/\"4\"[0-2]/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/${Zipcode} & ^${PrefixKY}/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"87111", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"41502", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"49502", false);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"abc", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"123", false);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502-123", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"40502-abc", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ReadmeExamples)
BOOST_AUTO_TEST_CASE(aeiou) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"h\"[aeiou]+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"ha", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"h", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"ae ", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"haec ", false);
}
BOOST_AUTO_TEST_CASE(aeiouascii) {
    auto texecutor = tryParseForASCIIDocsTest("/'h'[aeiou]+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_ASCII_DOCS(executor, "", false);
    ACCEPTS_TEST_ASCII_DOCS(executor, "ha", true);

    ACCEPTS_TEST_ASCII_DOCS(executor, "h", false);
    ACCEPTS_TEST_ASCII_DOCS(executor, "ae ", false);
    ACCEPTS_TEST_ASCII_DOCS(executor, "haec ", false);
}
BOOST_AUTO_TEST_CASE(aeiouspaces) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\n    \"h\" %%starts with h \n  %* comment *%  [aeiou]+ %%then aeiou\n/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"ha", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"h", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"ae ", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"haec ", false);
}
BOOST_AUTO_TEST_CASE(pepper) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"🌶\" %*unicode pepper*%/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌶🌶", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌶", true);
}
BOOST_AUTO_TEST_CASE(hexescapes) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"%x1f335; %x59;\" %*unicode pepper*%/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌵 ", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌵 Y", true);
}
BOOST_AUTO_TEST_CASE(commonescapes) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/\"%NUL; %n; %%; %;\" %* null, newline, literal %, and a \" quote*%/");
    BOOST_CHECK(texecutor.has_value());
}
BOOST_AUTO_TEST_CASE(escapesinrange) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/[🌵🌶]?/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌶🌶", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌶", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"🌵", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(repeatsnumber) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/[+-]? (\"0\" | [1-9][0-9]+)/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"+01", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"1234", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"1000", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"-1000", true);
}
BOOST_AUTO_TEST_CASE(repeatsfilename) {
    auto texecutor = tryParseForUnicodeDocsTest(u8"/[a-zA-Z0-9_]+ \".\" [a-zA-Z0-9]{1,3}/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.txt", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_1.pdf", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_abc_.g", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8".txt", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.t_t", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.pogo", false);
}
BOOST_AUTO_TEST_CASE(namednumber) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("NonZeroDigit", u8"/[1-9]/", nmap));
    BOOST_CHECK(tryParseIntoNameMap("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/[+-]? (\"0\" | ${NonZeroDigit}${Digit}+)/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"+01", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"1234", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"1000", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"-1000", true);
}
BOOST_AUTO_TEST_CASE(conjunctionfilehasext) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("Filename", u8"/[a-zA-Z0-9_]+ \".\" [a-zA-Z0-9]{1,}/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/${Filename} & \".txt\"$/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.txt", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_1.pdf", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_abc_.g", false);
}
BOOST_AUTO_TEST_CASE(conjunctionfilenotext) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("Filename", u8"/[a-zA-Z0-9_]+ \".\" [a-zA-Z0-9]{1,}/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/${Filename} & !(\".tmp\" | \".scratch\")$/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.txt", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_1.pdf", true);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_abc_.g", true);

    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"a.tmp", false);
    ACCEPTS_TEST_UNICODE_DOCS(executor, u8"_1.scratch", false);
}
BOOST_AUTO_TEST_CASE(anchorfile) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMap("FilenameFragment", u8"/[a-zA-Z0-9_]+/", nmap));

    auto texecutor = tryParseForNameSubTest(u8"/\"mark_\"^<${FilenameFragment}>$!(\".tmp\" | \".scratch\")/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"a.txt", 0, 3, false);
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"a.txt", 0, 0, false);
    
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mark_a.txt", 5, 5, true);
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mark_ab.txt", 5, 6, true);
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mark_a.txt", 5, 6, false);

    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mak_a.txt", 4, 4, false);
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mark_a.tmp", 5, 5, false);
    ACCEPTS_TEST_UNICODE_RNG_DOCS(executor, u8"mark_a.tmpa", 5, 5, false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
