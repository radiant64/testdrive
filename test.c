#include "testdrive.h"

#include <assert.h>
#include <stdbool.h>

FIXTURE(nested_sections, "Nested sections are visited in order")
    REQUIRE(true && "Top level assertion visibility test.");

    bool foo = true;
    bool bar = false;

    SECTION("Top level section A")
        REQUIRE(foo);
    END_SECTION

    SECTION("Top level section B")
        REQUIRE(!bar);
        bool baz = true;
        int i = 0;

        SECTION("Nested section B.A")
            SECTION("Nested section B.A.A")
                i++;
                REQUIRE(true && "Should always succeed.");
            END_SECTION

            do {
                REQUIRE(i == 0 && "State should be reset between iterations.");
                REQUIRE_FAIL(bar && "Should fail and block section B.A.B.");
            } while(1);

            SECTION("Nested section B.A.B")
                assert(false && "Should never be tested.");
            END_SECTION
        END_SECTION

        SECTION("Nested section B.B")
            REQUIRE(baz && "Nested scope visibility.");
        END_SECTION
    END_SECTION

    SECTION("Top level section C")
        REQUIRE(foo != bar);
    END_SECTION
END_FIXTURE

int main(int argc, char** argv) {
    return RUN_TEST(nested_sections);
}

