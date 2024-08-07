#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex_system.h"

BOOST_AUTO_TEST_SUITE(System)

BOOST_AUTO_TEST_SUITE(Single)
BOOST_AUTO_TEST_CASE(abc) {
    brex::RENSInfo ninfo = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo };
    std::vector<std::u8string> errors;
    brex::ReSystem::processSystem(ninfos, errors);

    BOOST_ASSERT(errors.empty());

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Chain)
BOOST_AUTO_TEST_CASE(abcxyz) {
    brex::RENSInfo ninfo = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            
            },
            {
                "Bar",
                u8"/\"xyz\"/"
            
            },
            {
                "Baz",
                u8"/${Foo} \"-\" ${Bar}/"
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo };
    std::vector<std::u8string> errors;
    auto sys = brex::ReSystem::processSystem(ninfos, errors);

    BOOST_CHECK(errors.empty());
    BOOST_CHECK(sys.getUnicodeRE("Main::Foo") != nullptr);
    BOOST_CHECK(sys.getUnicodeRE("Main::Baz") != nullptr);

    auto executor = sys.getUnicodeRE("Main::Baz");
    brex::UnicodeString ustr = u8"abc-xyz";
    brex::UnicodeString estr = u8"abc-123";
    brex::ExecutorError err = brex::ExecutorError::Ok;

    BOOST_ASSERT(executor->test(&ustr, err));
    BOOST_ASSERT(!executor->test(&estr, err));
}
BOOST_AUTO_TEST_CASE(twons) {
    brex::RENSInfo ninfo1 = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            
            },
            {
                "Bar",
                u8"/\"xyz\"/"
            
            },
            {
                "Baz",
                u8"/${Foo} \"-\" ${Bar}/"
            }
        }
    };
    brex::RENSInfo ninfo2 = {
        {
            "Other",
            { {"MM", "Main"} }
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            
            },
            {
                "Baz",
                u8"/${Foo} \"-\" ${MM::Foo}/"
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo1, ninfo2 };
    std::vector<std::u8string> errors;
    auto sys = brex::ReSystem::processSystem(ninfos, errors);

    BOOST_CHECK(errors.empty());
    BOOST_CHECK(sys.getUnicodeRE("Other::Foo") != nullptr);
    BOOST_CHECK(sys.getUnicodeRE("Other::Baz") != nullptr);

    auto executor = sys.getUnicodeRE("Other::Baz");
    brex::UnicodeString ustr = u8"abc-abc";
    brex::UnicodeString estr = u8"abc-123";
    brex::ExecutorError err = brex::ExecutorError::Ok;

    BOOST_ASSERT(executor->test(&ustr, err));
    BOOST_ASSERT(!executor->test(&estr, err));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(DAG)
BOOST_AUTO_TEST_CASE(abcabc) {
    brex::RENSInfo ninfo = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            
            },
            {
                "Baz",
                u8"/${Foo} \"-\" ${Main::Foo}/"
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo };
    std::vector<std::u8string> errors;
    auto sys = brex::ReSystem::processSystem(ninfos, errors);

    BOOST_CHECK(errors.empty());
    BOOST_CHECK(sys.getUnicodeRE("Main::Foo") != nullptr);
    BOOST_CHECK(sys.getUnicodeRE("Main::Baz") != nullptr);

    auto executor = sys.getUnicodeRE("Main::Baz");
    brex::UnicodeString ustr = u8"abc-abc";
    brex::UnicodeString estr = u8"abc-123";
    brex::ExecutorError err = brex::ExecutorError::Ok;

    BOOST_ASSERT(executor->test(&ustr, err));
    BOOST_ASSERT(!executor->test(&estr, err));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Cycle)
BOOST_AUTO_TEST_CASE(abcabc) {
    brex::RENSInfo ninfo = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/${Baz}/"
            
            },
            {
                "Baz",
                u8"/${Foo}/"
            
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo };
    std::vector<std::u8string> errors;
    auto sys = brex::ReSystem::processSystem(ninfos, errors);

    BOOST_ASSERT(!errors.empty());
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()