/** @file   collections.h
 *  @brief  provide functions related to collections.
 *
 *  provide collections (list, ...)
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#ifndef __HFSM_COLLECTIONS_H__
#define __HFSM_COLLECTIONS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    char payload[];
} LIST_NODE;

#define LIST_NODE_INITIALIZER \
    (LIST_NODE){              \
        .prev = NULL,         \
        .next = NULL          \
    }

typedef struct list {
    struct list_node *root;
    struct list_node *last;
    size_t payload_bytes;
    size_t length;
} LIST;

#define LIST_INITIALIZER(bytes)   \
    (LIST){                       \
        .root = NULL,             \
        .last = NULL,             \
        .payload_bytes = (bytes), \
        .length = 0               \
    }

typedef struct list2 {
    void *pool;
    struct list_node *released;
    struct list_node *root;
    struct list_node *last;
    size_t payload_bytes;
    size_t count;
} LIST2;

#define LIST2_INITIALIZER(bytes)  \
    (LIST2){                      \
        .pool = NULL,             \
        .released = NULL,         \
        .root = NULL,             \
        .last = NULL,             \
        .payload_bytes = (bytes), \
        .count = 0                \
    }

typedef LIST_NODE *LIST_ITER;

static inline void list2_push_released(LIST2 *list, LIST_NODE *node)
{
    node->next = list->released;
    list->released->prev = node;
    list->released = node;
}

static inline LIST_NODE *list2_pop_released(LIST2 *list)
{
    LIST_NODE *node = list->released;
    if (node == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    list->released = node->next;
    list->released->prev = NULL;
    return node;
}

static inline int list_init(LIST *list, size_t payload_bytes)
{
    if (list == NULL) {
        errno = EINVAL;
        return -1;
    }
    *list = LIST_INITIALIZER(payload_bytes);
    return 0;
}

static inline int list2_init(LIST2 *list, size_t payload_bytes, size_t capacity)
{
    LIST_NODE *node;
    size_t node_bytes;
    size_t i;

    if ((list == NULL) || (payload_bytes == 0) || (capacity == 0)) {
        errno = EINVAL;
        return -1;
    }

    *list = LIST2_INITIALIZER(payload_bytes);

    node_bytes = sizeof(LIST_NODE) + payload_bytes;
    list->pool = calloc(capacity, node_bytes);
    if (list->pool == NULL) {
        return -1;
    }

    list->released = (LIST_NODE *)list->pool;
DEBUG("node: %p", list->released);
    for (i = 1; i < capacity; ++i) {
        node = (LIST_NODE *)((uintptr_t)list->pool + (node_bytes * i));
DEBUG("node: %p", node);
        list2_push_released(list, node);
    }

    return 0;
}

static inline void list_release(LIST *list)
{
    LIST_NODE *node = list->root;
    LIST_NODE *next;

    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    list->length = 0;
}

static inline void list2_release(LIST2 *list)
{
    size_t payload_bytes = list->payload_bytes;
    free(list->pool);
    *list = LIST2_INITIALIZER(payload_bytes);
}

static inline void *list_add(LIST *list, void *payload)
{
    LIST_NODE *node = (LIST_NODE *)malloc(sizeof(LIST_NODE) + list->payload_bytes);
    if (node == NULL) {
        return NULL;
    }
    *node = LIST_NODE_INITIALIZER;
    memcpy(node->payload, payload, list->payload_bytes);

    if (list->root == NULL) {
        list->root = list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }
    ++list->length;

    return node->payload;
}

static inline void *list2_add(LIST2 *list, void *payload)
{
    LIST_NODE *node = list2_pop_released(list);
    if (node == NULL) {
        return NULL;
    }
    *node = LIST_NODE_INITIALIZER;
    memcpy(node->payload, payload, list->payload_bytes);

    if (list->root == NULL) {
        list->root = list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }
    ++list->count;

    return node->payload;
}

static inline void *list2_insert(LIST2 *list, int index, void *payload)
{
    LIST_NODE *node;
    LIST_NODE *iter;
    int i;

    node = list2_pop_released(list);
    if (node == NULL) {
        return NULL;
    }
    *node = LIST_NODE_INITIALIZER;
    memcpy(node->payload, payload, list->payload_bytes);

    if (index == 0) {
        if (list->root == NULL) {
            list->root = list->last = node;
        } else {
            node->next = list->root;
            list->root->prev = node;
            list->root = node;
        }
    } else if (index > 0) {
        iter = list->root;
        for (i = 0; i < index; ++i) {
            if (iter == NULL) {
                errno = EINVAL;
                return NULL;
            }
            iter = iter->next;
        }
        if (iter == NULL) {
            node->prev = list->last;
            list->last->next = node;
            list->last = node;
        } else {
            node->next = iter;
            node->prev = iter->prev;
            node->prev->next = node;
            iter->prev = node;
        }
    } else {
        if (list->last == NULL) {
            list->root = list->last = node;
        } else {
            node->prev = list->last;
            list->last->next = node;
            list->last = node;
        }
    }
    ++list->count;

    return node->payload;
}

static inline void list_remove(LIST *list, LIST_ITER iter)
{
    --list->length;
    if (list->root == iter) {
        list->root = iter->next;
    }
    if (list->last == iter) {
        list->last = iter->prev;
    }
    if (iter->prev != NULL) {
        iter->prev->next = iter->next;
    }
    if (iter->next != NULL) {
        iter->next->prev = iter->prev;
    }
    free(iter);
}

static inline void list2_remove(LIST2 *list, LIST_ITER iter)
{
    --list->count;
    if (list->root == iter) {
        list->root = iter->next;
    }
    if (list->last == iter) {
        list->last = iter->prev;
    }
    if (iter->prev != NULL) {
        iter->prev->next = iter->next;
    }
    if (iter->next != NULL) {
        iter->next->prev = iter->prev;
    }

    list2_push_released(list, iter);
}

static inline int list_push(LIST *list, void *payload)
{
    void *ret = list_add(list, payload);
    return (ret != NULL) ? 0 : -1;
}

static inline int list_pop(LIST *list, void *payload)
{
    LIST_NODE *node = list->last;

    if (node == NULL) {
        memset(payload, 0, list->payload_bytes);
        return -1;
    }

    --list->length;
    if (node->prev == NULL) {
        list->root = NULL;
        list->last = NULL;
    } else {
        list->last = node->prev;
        node->prev->next = NULL;
        node->prev = NULL;
    }
    memcpy(payload, node->payload, list->payload_bytes);
    free(node);

    return 0;
}

static inline LIST_ITER list_iter(LIST *list)
{
    return list->root;
}

static inline LIST_ITER list2_iter(LIST2 *list)
{
    return list->root;
}

static inline LIST_ITER list_next(LIST_ITER iter)
{
    return iter->next;
}

static inline void *list_get_payload(LIST_ITER iter)
{
    return iter->payload;
}

#endif /* __HFSM_COLLECTIONS_H__ */
