# testdrive

An expressive single-header unit testing framework for C (99 or later).

## Introduction

When writing any but the simplest unit tests, a lot of code typically deals
with setting up preconditions. There are several approaches to this problem,
the most common being employing some sort of _test fixtures_. These can aid
when writing many different tests that rely on a complex but similar state, but
the traditional approach where fixtures are data objects deals poorly with
testing sequences, where the state is mutating and evolving between the test
cases.

The approach taken in *testdrive* is inspired by
[Catch2](https://github.com/catchorg/Catch2). In it, test fixtures can contain
any number of nested _test sections_, which can extend and alter the state as
needed.

## Example

```c
#include "testdrive.h"

// Tentative file to test.
#include "config.h" 

FIXTURE(read_configuration, "Reading configuration")
    struct conf_type* config;
    conf_prepare_mock();
    REQUIRE(conf_is_mocking());

    SECTION("Reading configuration content succeeds")
        conf_mock_read_success();
        config = conf_read("conf_file");
        REQUIRE(config);

        SECTION("Accessing existing configuration field")
            const char* field = conf_read_field(config, "foo");
            REQUIRE(strcmp(field, "bar") == 0);
        END_SECTION
        
        SECTION("Accessing unknown configuration field")
            const char* field = conf_read_field(config, "bar");
            REQUIRE(!field);
        END_SECTION
    END_SECTION

    SECTION("Reading configuration content fails")
        conf_mock_read_failure();
        config = conf_read("conf_file");
        REQUIRE(!config);
    END_SECTION
END_FIXTURE

int main(int argc, char** argv) {
    return RUN_TEST(read_configuration);
}
```

