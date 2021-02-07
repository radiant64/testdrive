#pragma once

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
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

#define TD_EVENT(EVENT_)\
    td_listener(EVENT_, td__test_ptr, __FILE__, __LINE__)

#define TD_FIXTURE(NAME, DESCRIPTION)\
    struct td_test_context TD_TEST_INFO(NAME) = {\
        #NAME,\
        DESCRIPTION\
    };\
    void TD_TEST_FUNCTION(NAME)(struct td_test_context* td__test_ptr) {\
        TD_ALLOC_SECTIONS(td__test_ptr);\
        size_t td__assert_count = 0;\
        TD_EVENT(TD_TEST_START);\
        size_t td__sequence = 0;\
        jmp_buf td__begin;\
        jmp_buf* td__continue = &td__begin;\
        bool td__from_assert = false;\
        do {\
            do {\
                td__from_assert = false;\
                if (setjmp(td__begin) == 1) {\
                    td__sequence++;\
                    td__from_assert = true;\
                    if (td__sequence > td__test_ptr->section_idx) {\
                        break;\
                    }\
                }\
            } while (td__from_assert);\
            if (td__from_assert) {\
                td__from_assert = false;\
                break;\
            }\
            td__test_ptr->section_idx = 0;

#define TD_SECTION(DESCRIPTION)\
    TD_EVENT(TD_SECTION_PRE);\
    assert(td__test_ptr->section_idx < TD_MAX_SECTIONS);\
    if (\
        td__test_ptr->sections[++(td__test_ptr->section_idx)].name\
            == NULL\
    ) {\
        td__test_ptr->sections[td__test_ptr->section_idx]\
            = (struct td_test_context) {\
                "",\
                DESCRIPTION\
            };\
    }\
    if (td__sequence != td__test_ptr->section_idx) {\
        TD_EVENT(TD_SECTION_SKIP);\
    } else {\
        {\
            struct td_test_context* td__parent_ptr = td__test_ptr;\
            size_t td__parent_seq = td__sequence;\
            jmp_buf* td__parent_continue = td__continue;\
            td__test_ptr = &td__test_ptr->sections[td__sequence];\
            TD_ALLOC_SECTIONS(td__test_ptr);\
            TD_EVENT(TD_SECTION_START);\
            jmp_buf td__section_begin;\
            td__continue = &td__section_begin;\
            td__sequence = 0;\
            do {\
                do {\
                    td__from_assert = false;\
                    if (setjmp(td__section_begin) == 1) {\
                        td__sequence += 1;\
                        td__from_assert = true;\
                        if (td__sequence > td__test_ptr->section_idx) {\
                            break;\
                        }\
                    }\
                } while (td__from_assert);\
                if (td__from_assert) {\
                    td__from_assert = false;\
                    break;\
                }\
                td__test_ptr->section_idx = 0;

#define TD_END_SECTION\
            } while (td__sequence++ < td__test_ptr->section_idx);\
            TD_EVENT(TD_SECTION_END);\
            td__test_ptr = td__parent_ptr;\
            td__sequence = td__parent_seq;\
            td__continue = td__parent_continue;\
        }\
    }

#define TD_END_FIXTURE\
        } while (td__test_ptr->section_idx > td__sequence++);\
        TD_EVENT(TD_TEST_END);\
    }

#define TD_EXTERN_FIXTURE(NAME)\
    extern struct td_test TD_TEST_INFO(NAME);\
    void TD_TEST_FUNCTION(NAME)(struct td_test*, const size_t)

#define TD_REQUIRE(CONDITION)\
    td__test_ptr->current_assertion = #CONDITION;\
    TD_EVENT(TD_ASSERT_PRE);\
    if (!(CONDITION)) {\
        TD_EVENT(TD_ASSERT_FAILURE);\
        longjmp(*td__continue, 1);\
    }\
    TD_EVENT(TD_ASSERT_SUCCESS);

#define TD_SET_LISTENER(LISTENER_FUNC) td_listener = LISTENER_FUNC

#define TD_RUN_TEST(NAME) TD_TEST_FUNCTION(NAME)(&TD_TEST_INFO(NAME))

#ifndef TD_DEFAULT_LISTENER
#define TD_DEFAULT_LISTENER td_console_listener
#endif

#ifndef TD_ONLY_PREFIXED_MACROS

#define FIXTURE(...) TD_FIXTURE(__VA_ARGS__)
#define SECTION(...) TD_SECTION(__VA_ARGS__)
#define END_SECTION TD_END_SECTION
#define END_FIXTURE TD_END_FIXTURE
#define EXTERN_FIXTURE(...) TD_EXTERN_FIXTURE(__VA_ARGS__)

#define REQUIRE(...) TD_REQUIRE(__VA_ARGS__)

#define RUN_TEST(...) TD_RUN_TEST(__VA_ARGS__)

#endif

enum td_event {
    TD_TEST_START,
    TD_SECTION_PRE,
    TD_SECTION_SKIP,
    TD_SECTION_START,
    TD_SECTION_END,
    TD_ASSERT_PRE,
    TD_ASSERT_FAILURE,
    TD_ASSERT_SUCCESS,
    TD_TEST_END
};

struct td_test_context {
    const char* name;
    const char* description;
    size_t section_idx;
    struct td_test_context* sections;
    const char* current_assertion;
};

static void td_console_listener(
    enum td_event event,
    struct td_test_context* test,
    const char* file,
    size_t line
);

static void(*td_listener)(
    enum td_event event,
    struct td_test_context* test,
    const char* file,
    size_t line
) = TD_DEFAULT_LISTENER;

static void td_console_listener(
    enum td_event event,
    struct td_test_context* test,
    const char* file,
    size_t line
) {
    switch (event) {
    case TD_TEST_START:
        printf("TD_TEST_START\n");
        break;
    case TD_SECTION_PRE:
        printf("TD_SECTION_PRE\n");
        break;
    case TD_SECTION_SKIP:
        printf(
            "TD_SECTION_SKIP: %s\n",
            test->sections[test->section_idx].description
        );
        break;
    case TD_SECTION_START:
        printf("TD_SECTION_START: %s\n", test->description);
        break;
    case TD_SECTION_END:
        printf("TD_SECTION_END\n");
        break;
    case TD_ASSERT_PRE:
        printf("TD_ASSERT_PRE: %s\n", test->current_assertion);
        break;
    case TD_ASSERT_FAILURE:
        printf("TD_ASSERT_FAILURE\n");
        break;
    case TD_ASSERT_SUCCESS:
        printf("TD_ASSERT_SUCCESS\n");
        break;
    case TD_TEST_END:
        printf("TD_TEST_END\n");
        break;
    }
}

