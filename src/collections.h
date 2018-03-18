/** @file   collections.h
 *  @brief  provide functions related to collections.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#ifndef __HFSM_COLLECTIONS_H__
#define __HFSM_COLLECTIONS_H__

#include <stdio.h>
#include <stdlib.h>
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

typedef LIST_NODE *LIST_ITER;

static inline int list_init(LIST *list, size_t payload_bytes)
{
    if (list == NULL) {
        errno = EINVAL;
        return -1;
    }
    *list = LIST_INITIALIZER(payload_bytes);
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

static inline LIST_ITER list_next(LIST_ITER iter)
{
    return iter->next;
}

static inline void *list_get_payload(LIST_ITER iter)
{
    return iter->payload;
}

#endif /* __HFSM_COLLECTIONS_H__ */
