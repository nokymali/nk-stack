//
//  nk-stack.c
//  0011-Demo.C.nk-stack
//
//  ============explain==============
//  /**< which chunk, 0, 1, 2,.....*/
//
//  /**< inner chunk position, [0, stack->chunk_max]*/
//
//  [chunk01][0, 1, 2, 3, 4, 5, 6, ......stack->chunk_max) -> chunk0
//  [chunk02][0, 1, 2, 3, 4, 5, 6, ......stack->chunk_max) -> chunk1
//  [chunk03][0, 1, 2, 3, 4, 5, 6, ......stack->chunk_max) -> chunk2
//
//  stack->chunk_items: all of the stack count
//
//  Created by ma li on 2021/12/11.
//
#include <unistd.h>
#include <assert.h>
#include "nk-stack.h"


//#define TRACE

#define likely(x)  __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DEFAULT_CHUNK_COUNT 32

#define PTR_SIZE (sizeof(void *))
#define PAGE_SIZE (getpagesize())

#define NK_STACK_CURR_PTR(s) (((s)->chunk_curr) + (((s)->chunk_pos) * PTR_SIZE) + PTR_SIZE)
#define NK_STACK_PREV_PTR(s) (((s)->chunk_curr) + ((((s)->chunk_pos) - 1) * PTR_SIZE) + PTR_SIZE)

struct _nk_stack {
    uint8_t  mode;
    int chunk_size;                 /**< each chunk size*/
    int chunk_max;                  /**< each chunk include item count*/
    volatile int chunk_pos;         /**< a current chunk position*/
    int chunk_items;                /**< all chunk's items count*/
    int chunk_count;                /**< alloc chunk's count*/
    void *chunk_curr;               /**< current chunk ptr, look like single linked list*/
    void *chunk_recycle;            /**< can re-used chunk*/
};

static inline __attribute__((always_inline)) nk_stack *nk_stack_create_option(int size, int mode) {

    nk_stack *stack = (nk_stack *)calloc(1, sizeof(nk_stack));
    if (unlikely(stack == NULL)) {
        assert("Fail to create nk_stack.\n");
        return NULL;
    }
    if (size < (DEFAULT_CHUNK_COUNT * PTR_SIZE))
        size = DEFAULT_CHUNK_COUNT * PTR_SIZE;

    /*
     * for test multiple chunks. so close it
    int pgsz = getpagesize();
    if (size % pgsz)
        size += (pgsz - (size % pgsz));
    */

    /**< multiple of PTR_SIZE align with PTR_SIZE*/
    if (size % PTR_SIZE)
        size += (PTR_SIZE - (size % PTR_SIZE));
    
    stack->mode             = mode;
    stack->chunk_size       = size + PTR_SIZE;
    stack->chunk_max        = size / PTR_SIZE;
    stack->chunk_pos        = 0;
    stack->chunk_count      = 0;
    stack->chunk_items      = 0;
    stack->chunk_curr       = NULL;
    stack->chunk_recycle    = NULL;
    return stack;
}

#if 0
static inline void debug_info(void *chunk, char *name) {
    char buffer[2048] = {'\0'};
    int sz = 0, idx = 0;
    void *p = NULL;
    void *recycle = chunk;
    printf("{Debug Output %s Start}\n", name);
    while ((p = recycle)) {
        sz += snprintf(buffer + sz, 2048, "\tchunk[%02d]: %p\n", idx++, p);
        recycle = *(void **)recycle;
    }
    printf("%s", buffer);
    printf("{Debug Output %s End}\n", name);
}
#endif

nk_stack *nk_stack_auto_create(int size) {
    return nk_stack_create_option(size, MODE_NK_STACK_AUTO);
}

nk_stack *nk_stack_fixed_create(int size) {
    return nk_stack_create_option(size, MODE_NK_STACK_FIXED);
}

int nk_stack_destroy(nk_stack *stack) {
    if (unlikely(stack == NULL))
        return 0;

    void *ptr = NULL;
    /**< destroy current chunks*/
    while ((ptr = stack->chunk_curr)) {
        printf("Destroy current chunk[%02d] %p\n", stack->chunk_count, ptr);
        stack->chunk_curr = *(void **)ptr;
        free(ptr);
        ptr = NULL;
        stack->chunk_count -= 1;
    }
    
    /**< destroy recycle chunks*/
    while ((ptr = stack->chunk_recycle)) {
        printf("Destroy recycle chunk[%02d] %p\n", stack->chunk_count, ptr);
        stack->chunk_recycle = *(void **)ptr;
        free(ptr);
        ptr = NULL;
        stack->chunk_count -= 1;
    }
    free(stack);
    stack = NULL;
    printf("Success to destroy nk_stack\n");
    return 1;
}

int nk_stack_is_empty(nk_stack * const stack) {
    return stack->chunk_items == 0;
}

int nk_stack_is_full(nk_stack * const stack) {
    if (stack->mode & MODE_NK_STACK_AUTO)
        return 0;
    return stack->chunk_pos == stack->chunk_max;;
}

int nk_stack_push(nk_stack * const stack, void *item) {
    if (unlikely(stack == NULL || item == NULL))
        return 0;

    /**< check current chunk */
    if (NULL == stack->chunk_curr) {
        /**< check recycle chunk*/
        if (NULL == stack->chunk_recycle) {
            /**< recycle chunk is NULL, so create new chunk*/
            void *chunk = (void *)calloc(stack->chunk_size, PTR_SIZE);
            *(void **)chunk = stack->chunk_curr;
            stack->chunk_curr = chunk;
            stack->chunk_count += 1;
        } else {
            /**< get a empty chunk from recycle chunks*/

            /**< 1. erase recycle chunks*/
            void *chunk = stack->chunk_recycle;
            stack->chunk_recycle = *(void **)stack->chunk_recycle;
            
            /**<2. add current chunks*/
            *(void **)chunk = stack->chunk_curr;
            stack->chunk_curr = chunk;
        }
        stack->chunk_pos = 0;
    }
    
    /**< push element to current chunk*/
    /**< void *store = stack->chunk_curr + (stack->chunk_pos * PTR_SIZE) + PTR_SIZE;*/
    void *store = NK_STACK_CURR_PTR(stack);

    *(void **)store = item;
    stack->chunk_pos += 1;
    stack->chunk_items += 1;
#if TRACE
    printf("nk_stack {Push}. chunk[%02d]: %p, pos: %p, index: %02d, element[%02d]: %p\n",
           stack->chunk_count, stack->chunk_curr, store, stack->chunk_items, stack->chunk_pos, item);
#endif

    /**< current chunk is full. get new chunk to store item*/
    if (stack->chunk_pos == stack->chunk_max) {

        /**< get exist chunk from recycle chunks*/
        if (NULL != stack->chunk_recycle) {

            /**< 1. erase recycle chunks*/
            void *chunk = stack->chunk_recycle;
            stack->chunk_recycle = *(void **)stack->chunk_recycle;
            
            /**<2. add current chunks*/
            *(void **)chunk = stack->chunk_curr;
            stack->chunk_curr = chunk;
        } else {

            /**< alloc new chunk*/
            void *chunk = (void *)calloc(stack->chunk_size, PTR_SIZE);
            *(void **)chunk = stack->chunk_curr;
            stack->chunk_curr = chunk;
            stack->chunk_count += 1;
        }
        stack->chunk_pos = 0;
    }
    return 1;
}

void *nk_stack_pop(nk_stack * const stack) {
    if (unlikely(stack == NULL))
        return NULL;

    if (unlikely(stack->chunk_curr == NULL))
        return NULL;

    /**< get item from current chunk*/
    /**< void *item = stack->chunk_curr + ((stack->chunk_pos - 1) * PTR_SIZE) + PTR_SIZE; */
    void *item = NK_STACK_PREV_PTR(stack);
#if TRACE
    printf("nk_stack { Pop}. chunk[%02d]: %p, pos: %p, index: %02d, item[%02d]: %p\n",
           stack->chunk_count, stack->chunk_curr, item, stack->chunk_items, stack->chunk_pos, *(void **)item);
#endif
    stack->chunk_pos -= 1;
    stack->chunk_items -= 1;

    /**< current chunk is empty*/
    if (stack->chunk_pos == 0) {

        /**< 1. erase chunk from current chunks*/
        void *chunk = stack->chunk_curr;
        stack->chunk_curr = *(void **)stack->chunk_curr;

        /**< 2. add chunk to recycle chunk*/
        *(void **)chunk = stack->chunk_recycle;
        stack->chunk_recycle = chunk;
        stack->chunk_pos = stack->chunk_max;
    }
    return *(void **)item;
}

void *nk_stack_peek(nk_stack * const stack) {
    if (NULL == stack || NULL == stack->chunk_curr)
        return NULL;

    /**< nk-stack top item*/
    /**< void *item = stack->chunk_curr + ((stack->chunk_pos - 1) * PTR_SIZE) + PTR_SIZE;*/
    void *item = NK_STACK_PREV_PTR(stack);

#if TRACE
    printf("nk_stack {Peek}. chunk[%02d]: %p, pos: %p, index: %02d, item[%02d]: %p\n",
           stack->chunk_count, stack->chunk_curr, item, stack->chunk_items, stack->chunk_pos, *(void **)item);
#endif

    return *(void **)item;
}

void nk_stack_info(nk_stack * const stack) {
    printf("==============================nk stack info begin==============================\n");
    printf("stack mode                  : %s\n", stack->mode & MODE_NK_STACK_AUTO ? "Auto" : "Fixed");
    printf("stack max                   : %d\n", stack->chunk_max);
    printf("stack current position      : %d\n", stack->chunk_pos);
    printf("stack elements count        : %"PRIu32"\n", stack->chunk_items);
    printf("stack chunks count          : %d\n", stack->chunk_count);
    printf("stack chunk ptr:\n");
    int cnt = 0;
    void *chunk = stack->chunk_curr;
    void *ptr = NULL;
    while ((ptr = chunk)) {
        chunk = *(void **)chunk;
        printf("\tchunk[%02d], ptr: %p, start: %p\n", cnt++, ptr, ptr + PTR_SIZE);
    }
    chunk = stack->chunk_recycle;
    while ((ptr = chunk)) {
        chunk = *(void **)chunk;
        printf("\tchunk[%02d], ptr: %p, start: %p\n", cnt++, ptr, ptr + PTR_SIZE);
    }
    printf("==============================nk stack info end==============================\n");
}
