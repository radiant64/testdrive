#include "testdrive.h"

#include <assert.h>
#include <stdbool.h>

FIXTURE(test_nested_sections, "Nested sections are visited in order")
    REQUIRE(true && "Top level assertion visibility test.");

    bool a = true;
    bool b = false;

    SECTION("Top level section A")
        REQUIRE(a);
    END_SECTION

    SECTION("Top level section B")
        REQUIRE(!b);
        bool c = true;

        SECTION("Nested section B.A")
            REQUIRE(b && "This assertion should fail!");
            SECTION("Nested section B.A.A")
                assert(false && "Should never be tested.");
            END_SECTION
        END_SECTION

        SECTION("Nested section B.B")
            REQUIRE(c && "Nested scope visibility.");
        END_SECTION
    END_SECTION

    SECTION("Top level section C")
        REQUIRE(a != b);
    END_SECTION
END_FIXTURE(test_nested_sections)


int main(int argc, char** argv) {
    DEFAULT_RUN_TEST(test_nested_sections);
    return 0;
}

