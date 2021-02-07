#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TD_MAX_SECTIONS
#define TD_MAX_SECTIONS 128
#define TD_MAX_ASSERTS 256
#endif

#define TD_TEST_INFO(NAME) td__test_ ## NAME
#define TD_TEST_FUNCTION(NAME) td__test_fn_ ## NAME

#define TD_ALLOC_SECTIONS(CONTEXT)\
    if (!CONTEXT->sections) {\
        CONTEXT->sections = malloc(\
            sizeof(struct td_test_context) * TD_MAX_SECTIONS\
        );\
    }

#define TD_FIXTURE(NAME, DESCRIPTION)\
    struct td_test_context TD_TEST_INFO(NAME) = {\
        #NAME,\
        DESCRIPTION\
    };\
    void TD_TEST_FUNCTION(NAME)(\
        struct td_test_context* td__test_ptr,\
        size_t td__sequence\
    ) {\
        TD_ALLOC_SECTIONS(td__test_ptr);\
        td__test_ptr->section_idx = 0;\
        size_t td__assert_count = 0;\
        do {

#define TD_SECTION(DESCRIPTION)\
    assert(td__test_ptr->section_idx < TD_MAX_SECTIONS);\
    if (\
        td__test_ptr->sections[++td__test_ptr->section_idx].name\
        != td__dummy\
    ) {\
        td__test_ptr->sections[td__test_ptr->section_idx]\
            = (struct td_test_context) {\
                td__dummy,\
                DESCRIPTION\
            };\
    }\
    if (td__sequence != td__test_ptr->section_idx) {\
    } else {\
        {\
            struct td_test_context* td__parent_ptr = td__test_ptr;\
            size_t td__parent_seq = td__sequence;\
            size_t td__parent_assert_count = td__assert_count;\
            td__test_ptr = &td__test_ptr->sections[td__sequence];\
            TD_ALLOC_SECTIONS(td__test_ptr);\
            td__sequence = 0;\
            do {\
                td__assert_count = 0;\
                td__test_ptr->section_idx = 0;\

#define TD_END_SECTION\
            } while (td__sequence++ < td__test_ptr->section_idx);\
            td__test_ptr = td__parent_ptr;\
            td__sequence = td__parent_seq;\
            td__assert_count = td__parent_assert_count;\
        }\
    }

#define TD_END_FIXTURE(NAME)\
        } while (0);\
        if (td__test_ptr->section_idx > td__sequence) {\
            TD_TEST_FUNCTION(NAME)(td__test_ptr, td__sequence + 1);\
        }\
    }

#define TD_EXTERN_FIXTURE_SINGLE(NAME)\
    extern struct td_test TD_TEST_INFO(NAME);\
    void TD_TEST_FUNCTION(NAME)(struct td_test*, const size_t)

#define TD_EXTERN_FIXTURE(NAME, ...)\
    TD_EXTERN_FIXTURE_SINGLE(NAME);\
    TD_EXTERN_FIXTURE(__VA_ARGS__)

#define TD_REQUIRE(CONDITION)\
    if (td__test_ptr->total_count < ++td__assert_count) {\
        assert(td__test_ptr->total_count <= TD_MAX_ASSERTS);\
        td__test_ptr->asserts[td__test_ptr->total_count++] = #CONDITION;\
        if (!(CONDITION)) {\
            td__test_ptr->failed_asserts[td__test_ptr->failed_count++]\
                = td__test_ptr->total_count - 1;\
            break;\
        }\
    }

#define TD_REPORTER(REPORTER_FUNC, TEST_) REPORTER_FUNC(&TD_TEST_INFO(TEST_))

#define TD_DEFAULT_REPORTER(TEST_) TD_REPORTER(td_console_reporter, TEST_)

#define TD_RUN_TEST(NAME) TD_TEST_FUNCTION(NAME)(&TD_TEST_INFO(NAME), 0)

#define TD_REPORTER_RUN_TEST(REPORTER_, TEST_)\
    TD_RUN_TEST(TEST_);\
    REPORTER_(TEST_)

#define TD_DEFAULT_RUN_TEST(TEST_)\
    TD_REPORTER_RUN_TEST(TD_DEFAULT_REPORTER, TEST_)

#ifndef TD_ONLY_PREFIXED_MACROS

#define TEST_INFO(...) TD_TEST_INFO(__VA_ARGS__)
#define TEST_FUNCTION(...) TD_TEST_FUNCTION(__VA_ARGS__)

#define FIXTURE(...) TD_FIXTURE(__VA_ARGS__)
#define SECTION(...) TD_SECTION(__VA_ARGS__)
#define END_SECTION TD_END_SECTION
#define END_FIXTURE(...) TD_END_FIXTURE(__VA_ARGS__)
#define EXTERN_FIXTURE(...) TD_EXTERN_FIXTURE(__VA_ARGS__)

#define REQUIRE(...) TD_REQUIRE(__VA_ARGS__)

#define REPORTER(...) TD_REPORTER(__VA_ARGS__)
#define DEFAULT_REPORTER(...) TD_DEFAULT_REPORTER(__VA_ARGS__)

#define RUN_TEST(...) TD_RUN_TEST(__VA_ARGS__)
#define REPORTER_RUN_TEST(...) TD_REPORTER_RUN_TEST(__VA_ARGS__)
#define DEFAULT_RUN_TEST(...) TD_DEFAULT_RUN_TEST(__VA_ARGS__)

#endif

static const char* td__dummy = "DUMMY";

struct td_test_context {
    const char* name;
    const char* description;
    size_t total_count;
    size_t failed_count;
    size_t section_idx;
    struct td_test_context* sections;
    const char* asserts[TD_MAX_ASSERTS];
    size_t failed_asserts[TD_MAX_ASSERTS];
};

static void td_console_reporter_recurse(
    const struct td_test_context* test,
    const char* indentation
) {
    fprintf(stdout, "%sTest: %s\n", indentation, test->description);
    fprintf(
         stdout,
         "%s- %ld/%ld assertions failed\n",
         indentation,
         test->failed_count,
         test->total_count
    );
    char *section_ind = NULL;
    for (size_t i = 1; i <= test->section_idx; ++i) {
        if (!section_ind) {
            const size_t indlen = strlen(indentation);
            section_ind = malloc(indlen + 3);
            memcpy(section_ind, indentation, indlen);
            section_ind[indlen] = ' ';
            section_ind[indlen + 1] = ' ';
            section_ind[indlen + 2] = '\0';
        }
        td_console_reporter_recurse(&test->sections[i], section_ind);
    }
    free(section_ind);
}

static void td_console_reporter(const struct td_test_context* test) {
    td_console_reporter_recurse(test, "");
}

