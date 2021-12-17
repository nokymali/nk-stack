//
//  nk-stack.h
//  0011-Demo.C.nk-stack
//
//  Created by ma li on 2021/12/11.
//

#ifndef nk_stack_h
#define nk_stack_h
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

/**< nk-stack automatic mode, the capacity can auto increase, this mode is default.*/
#define MODE_NK_STACK_AUTO  0x0001

/**< nk-stack fixed mode，the capacity is fixed*/
#define MODE_NK_STACK_FIXED 0x0010

typedef struct _nk_stack nk_stack;

/**
 * nk-stack auto mode create
 * @param size nk-stack capacity size
 * @return nk-stack object
 */
nk_stack *nk_stack_auto_create(int size);

/**
 * nk-stack fixed mode create
 * @param size nk-stack capacity size
 * @return nk-stack object
 */
nk_stack *nk_stack_fixed_create(int size);

/**
 * destroy nk-stack object.
 *
 * [note]: here not responsible for destroying elements in the stack.
 *         please destroy by yourself.
 *
 * @param stack nk-stack object
 * @return 1: success
 *         0: failure
 */
int nk_stack_destroy(nk_stack *stack);

/**
 * judgement nk-stack is empty or not.
 * @param stack nk-stack object
 * @return 1: the nk-stack is empty.
 *         0: the nk-stack is not empty.
 */
int nk_stack_is_empty(nk_stack * const stack);

/**
 * judgement nk-stack object is full or not.
 * @param stack nk-stack object
 * @return MODE_NK_STACK_FIXED mode always zero.
 *         MODE_NK_STACK_FIXED mode:
 *          1: nk-stack is full.
 *          0: nk-stack is not full.
 */
int nk_stack_is_full(nk_stack * const stack);

/**
 * nk-stack push element.
 * @param stack nk-stack object.
 * @param element element object.
 * @return 1: push element success.
 *         0: push element failure.
 */
int nk_stack_push(nk_stack * const stack, void *element);

/**
 * pop element from nk-stack.
 * @param stack nk-stack object.
 * @return element.
 */
void *nk_stack_pop(nk_stack * const stack);

/**
 * peek element from nk-stack obejct.
 * @param stack nk-stack object.
 * @return top of nk-stack element.
 */
void *nk_stack_peek(nk_stack * const stack);

/**
 * display nk-stack information.
 * @param stack nk-stack object.
 */
void nk_stack_info(nk_stack * const stack);


#endif /* nk_stack_h */
