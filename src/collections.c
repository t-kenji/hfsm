/** @file   collections.c
 *  @brief  provide functions related to collections.
 *
 *  provide collections (list, stack, ...)
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "collections.h"

struct list_node {
    struct list_node *prev;
    struct list_node *next;
    char payload[];
};

#define LIST_NODE_INITIALIZER \
    (struct list_node){       \
        .prev = NULL,         \
        .next = NULL          \
    }

struct list {
    void *pool;
    struct list_node *released;
    struct list_node *root;
    struct list_node *last;
    size_t payload_bytes;
    size_t capacity;
    size_t count;
};

#define LIST_INITIALIZER(p, b, c) \
    (struct list){                \
        .pool = (p),              \
        .released = NULL,         \
        .root = NULL,             \
        .last = NULL,             \
        .payload_bytes = (b),     \
        .capacity = (c),          \
        .count = 0                \
    }

static inline void list_push_released(struct list *l, struct list_node *node)
{
    node->prev = NULL;
    if (l->released == NULL) {
        node->next = NULL;
    } else {
        node->next = l->released;
        l->released->prev = node;
    }
    l->released = node;
}

static inline struct list_node *list_pop_released(struct list *l)
{
    struct list_node *node = l->released;

    if (node == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    l->released = node->next;
    if (l->released != NULL) {
        l->released->prev = NULL;
    }

    return node;
}

static inline void list_setup(struct list *l,
                              void *pool,
                              size_t payload_bytes,
                              size_t capacity)
{
    size_t node_bytes;
    struct list_node *node;
    size_t i;

    *l = LIST_INITIALIZER(pool, payload_bytes, capacity);
    node_bytes = sizeof(struct list_node) + l->payload_bytes;
    for (i = 0; i < l->capacity; ++i) {
        node = (struct list_node *)((uintptr_t)l->pool + (node_bytes * i));
        list_push_released(l, node);
    }
}

LIST list_init(size_t payload_bytes, size_t capacity)
{
    struct list *l;
    size_t node_bytes;
    void *pool;

    if ((payload_bytes == 0) || (capacity == 0)) {
        errno = EINVAL;
        return NULL;
    }

    l = malloc(sizeof(struct list));
    node_bytes = sizeof(struct list_node) + payload_bytes;
    pool = calloc(capacity, node_bytes);
    if ((l == NULL) || (pool == NULL)) {
        free(pool);
        free(l);
        return NULL;
    }

    list_setup(l, pool, payload_bytes, capacity);

    return (LIST)l;
}

void list_release(LIST list)
{
    struct list *l = (struct list *)list;

    if (l != NULL) {
        free(l->pool);
        free(l);
    }
}

int list_clear(LIST list)
{
    struct list *l = (struct list *)list;

    if (l == NULL) {
        errno = EINVAL;
        return -1;
    }

    list_setup(l, l->pool, l->payload_bytes, l->capacity);

    return 0;
}

void *list_insert(LIST list, int index, void *payload)
{
    struct list *l = (struct list *)list;
    struct list_node *node;
    struct list_node *iter;
    int i;

    if ((l == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return NULL;
    }

    node = list_pop_released(l);
    if (node == NULL) {
        return NULL;
    }
    *node = LIST_NODE_INITIALIZER;
    memcpy(node->payload, payload, l->payload_bytes);

    if (index == 0) {
        if (l->root == NULL) {
            l->root = l->last = node;
        } else {
            node->next = l->root;
            l->root->prev = node;
            l->root = node;
        }
    } else if (index > 0) {
        iter = l->root;
        for (i = 0; i < index; ++i) {
            if (iter == NULL) {
                errno = EINVAL;
                return NULL;
            }
            iter = iter->next;
        }
        if (iter == NULL) {
            node->prev = l->last;
            l->last->next = node;
            l->last = node;
        } else {
            node->next = iter;
            node->prev = iter->prev;
            node->prev->next = node;
            iter->prev = node;
        }
    } else {
        if (l->last == NULL) {
            l->root = l->last = node;
        } else {
            node->prev = l->last;
            l->last->next = node;
            l->last = node;
        }
    }
    ++l->count;

    return node->payload;
}

void *list_add(LIST list, void *payload)
{
    return list_insert(list, -1, payload);
}

int list_remove(LIST list, LIST_ITER iter)
{
    struct list *l = (struct list *)list;
    struct list_node *node;

    if ((l == NULL) || (iter == NULL)) {
        errno = EINVAL;
        return -1;
    }

    node = (struct list_node *)iter;
    --l->count;
    if (l->root == node) {
        l->root = node->next;
    }
    if (l->last == node) {
        l->last = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    list_push_released(l, node);

    return 0;
}

LIST_ITER list_iter(LIST list)
{
    struct list *l = (struct list *)list;
    if (l == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (LIST_ITER)l->root;
}

LIST_ITER list_next(LIST_ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (LIST_ITER)((struct list_node *)iter)->next;
}

void *list_get_payload(LIST_ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return ((struct list_node *)iter)->payload;
}

STACK stack_init(size_t payload_bytes, size_t capacity)
{
    return (STACK)list_init(payload_bytes, capacity);
}

void stack_release(STACK stack)
{
    list_release((LIST)stack);
}

int stack_clear(STACK stack)
{
    return list_clear((LIST)stack);
}

void *stack_push(STACK stack, void *payload)
{
    return list_insert((LIST)stack, 0, payload);
}

int stack_pop(STACK stack, void *payload)
{
    struct list *l = (struct list *)stack;
    LIST_ITER iter = list_iter((LIST)l);

    if ((iter == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return -1;
    }

    memcpy(payload, list_get_payload(iter), l->payload_bytes);
    list_remove((LIST)l, iter);

    return l->count;
}
