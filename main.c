//
//  main.c
//  for test nk-stack.
//
//

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "nk-stack.h"

#define STACK_DEFAULT_SIZE 99
#define NAME_LENGHT 32

#ifndef TRACE
#define DISPLAY
#endif

static const char * const const_string = { "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
static int const_string_len = 0;

typedef struct _Data {
    int value;
    char name[32];
} __attribute__((__packed__)) Data;

static Data *newData();

int main(int argc, const char * argv[]) {
    const_string_len = strlen(const_string);
    srand((unsigned int)time(NULL));

    nk_stack *stack = nk_stack_auto_create(STACK_DEFAULT_SIZE);

    int i = 0;
    /**< push element*/
    for (i = 0; i < STACK_DEFAULT_SIZE; ++i) {
        Data *d = newData();
        int res = nk_stack_push(stack, d);
        if (0 == res) {
            printf("fail to push %p, value: %03d\n", d, d->value);
            free(d);
        } else {
#ifdef DISPLAY
            printf("nk_stack [push]: %p, %03d, %s\n", d, d->value, d->name);
#endif
        }
    }


    nk_stack_info(stack);

    void *v = nk_stack_peek(stack);
    if (v != NULL) {
#ifdef DISPLAY
        Data *d = (Data *)v;
        printf("nk_stack [peek]: %p, %03d, %s\n", d, d->value, d->name);
#endif
    }


    /**< pop*/
    int cnt = STACK_DEFAULT_SIZE >> 1;
    for (i = 0; i < cnt; ++i) {
        Data *d = nk_stack_pop(stack);
        if (d != NULL) {
#ifdef DISPLAY
           printf("nk-stack [pop]: %p, %03d, %s\n", d, d->value, d->name);
#endif
           free(d);
        }
    }

    /**< push again*/
    for (i = cnt + 1; i < STACK_DEFAULT_SIZE; ++i) {
        Data *d = newData();
        int res = nk_stack_push(stack, d);
        if (0 == res) {
            printf("fail to push %p, value: %03d\n", d, d->value);
            free(d);
        } else {
#ifdef DISPLAY
            printf("nk-stack [push]: %p, %03d, %s\n", d, d->value, d->name);
#endif
        }
    }

    /**< pop more than STACK_DEFAULT_SIZE*/
    for (i = 0; i < 1000; ++i) {
        void *v = nk_stack_pop(stack);

        if (v != NULL) {
#ifdef DISPLAY
            Data *d = (Data *) v;
            printf("nk-stack [pop]: %p, %03d, %s\n", d, d->value, d->name);
#endif
            free(v);
        }
    }

    printf("=============try again push=========\n");
    /**< push*/
    for (i = 0; i < STACK_DEFAULT_SIZE >> 1; ++i) {
        Data *d = newData();
        int res = nk_stack_push(stack, d);
        if (0 == res) {
            printf("fail to push %p, value: %03d\n", d, d->value);
            free(d);
        } else {
#ifdef DISPLAY
            printf("nk-stack [push]: %p, %03d, %s\n", d, d->value, d->name);
#endif
        }
    }

    while (!nk_stack_is_empty(stack)) {
        void *v = nk_stack_pop(stack);

        if (v != NULL) {
#ifdef DISPLAY
            Data *d = (Data *)v;
            printf("nk-stack destroy [pop]: %p, %03d, %s\n", d, d->value, d->name);
#endif
            free(v);
        }
    }
    nk_stack_destroy(stack);
    return 0;
}


inline __attribute__((always_inline)) Data *newData() {
    Data *d = (Data *)calloc(1, sizeof(Data));
    if (d != NULL) {
        d->value = rand() % 1023;

        int i = 0;
        for (i = 0; i < NAME_LENGHT - 1; ++i) {
            d->name[i] = const_string[rand() % const_string_len];
        }
        /**< Note. if there is no terminator characters,
         * there will be a warning in printf characters
         * （valgrind prompts: Invalid read of size 1）*/
        d->name[i] = '\0';
    }

    return d;
}
