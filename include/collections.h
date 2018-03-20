/** @file   collections.h
 *  @brief  provide functions related to collections.
 *
 *  provide collections (list, stack, ...)
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#ifndef __HFSM_COLLECTIONS_H__
#define __HFSM_COLLECTIONS_H__

#include <stdlib.h>

/**
 *  @addgroup CAT_LIST list
 *  @{
 */
typedef struct {} *LIST;

typedef struct {} *LIST_ITER;

LIST list_init(size_t payload_bytes, size_t capacity);

void list_release(LIST list);

int list_clear(LIST list);

void *list_add(LIST list, void *payload);

void *list_insert(LIST list, int index, void *payload);

int list_remove(LIST list, LIST_ITER iter);

LIST_ITER list_iter(LIST list);

LIST_ITER list_next(LIST_ITER iter);

void *list_get_payload(LIST_ITER iter);

/** @} */

/**
 *  @addgroup CAT_STACK stack
 *  @{
 */

typedef struct {} *STACK;

STACK stack_init(size_t payload_bytes, size_t capacity);

void stack_release(STACK stack);

int stack_clear(STACK stack);

void *stack_push(STACK stack, void *payload);

int stack_pop(STACK stack, void *payload);

/** @} */

#endif /* __HFSM_COLLECTIONS_H__ */
