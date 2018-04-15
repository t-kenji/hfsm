/** @file   collections.c
 *  @brief  コレクションに関する機能を提供する.
 *
 *  コレクション (リスト, スタック, キュー, ツリー) を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "collections.h"
#include "debug.h"

/**
 *  リストノード構造体.
 */
struct list_node {
    struct list_node *prev; /**< 前要素へのポインタ. */
    struct list_node *next; /**< 次要素へのポインタ. */
    char payload[];         /**< データ部. */
};

/**
 *  リストノード構造体の初期化子.
 */
#define LIST_NODE_INITIALIZER \
    (struct list_node){       \
        .prev = NULL,         \
        .next = NULL          \
    }

/**
 *  リスト管理構造体.
 */
struct list {
    void *pool;                 /**< リストで使用するメモリプール. */
    struct list_node *released; /**< 解放済みノードのリスト. */
    struct list_node *root;     /**< 使用中の先頭ノード. */
    struct list_node *last;     /**< 使用中の末尾ノード. */
    size_t payload_bytes;       /**< データ部のサイズ. */
    size_t capacity;            /**< 確保したノードの数. */
    size_t count;               /**< 使用中のノードの数. */
};

/**
 *  リスト管理構造体の初期化子.
 */
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

/**
 *  解放済みノードのリストにノードを追加する.
 *
 *  @param  [in,out]    self    リストオブジェクト.
 *  @param  [in]        node    追加するノード.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 *  @pre    @c node の非 NULL は呼び出し側で保証すること.
 */
static inline void list_push_released(struct list *self, struct list_node *node)
{
    node->prev = NULL;
    if (self->released == NULL) {
        node->next = NULL;
    } else {
        node->next = self->released;
        self->released->prev = node;
    }
    self->released = node;
}

/**
 *  解放済みノードのリストからノードを取得する.
 *
 *  @param  [in,out]    self    リストオブジェクト.
 *  @return 成功時は, ノードのポインタが返る.
 *          失敗時は, NULL が返り, errno が適切に設定される.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 */
static inline struct list_node *list_pop_released(struct list *self)
{
    struct list_node *node = self->released;

    if (node == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    self->released = node->next;
    if (self->released != NULL) {
        self->released->prev = NULL;
    }

    return node;
}

/**
 *  リストの初期設定を行う.
 *
 *  @param  [in,out]    self            リストオブジェクト.
 *  @param  [in]        pool            リストに使用するメモリプール.
 *  @param  [in]        payload_bytes   データ部のサイズ.
 *  @param  [in]        capacity        リストの容量.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 *  @pre    @c pool の非 NULL は呼び出し側で保証すること.
 */
static inline void list_setup(struct list *self,
                              void *pool,
                              size_t payload_bytes,
                              size_t capacity)
{
    size_t node_bytes;
    struct list_node *node;
    size_t i;

    *self = LIST_INITIALIZER(pool, payload_bytes, capacity);
    node_bytes = sizeof(*node) + self->payload_bytes;
    for (i = 0; i < self->capacity; ++i) {
        node = (struct list_node *)((uintptr_t)self->pool + (node_bytes * i));
        list_push_released(self, node);
    }
}

/**
 *  @details    @c iter の次の反復子を取得する.
 *
 *  @param      [in]    iter    リストの反復子.
 *  @return     成功時は, 次の反復子が返る. 次の要素がない場合は NULL が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ITER iter_next(ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (ITER)((struct list_node *)iter)->next;
}

/**
 *  @details    @c iter のデータ部を取得する.
 *
 *  @param      [in]    iter    リストの反復子.
 *  @return     成功時は, データ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *iter_get_payload(ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return ((struct list_node *)iter)->payload;
}

/**
 *  @details    空で, 指定の容量を備えた, @ref LIST オブジェクトを確保
 *              および初期化する.
 *
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        リストの容量.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
LIST list_init(size_t payload_bytes, size_t capacity)
{
    struct list *self;
    size_t node_bytes;
    void *pool;

    if ((payload_bytes == 0) || (capacity == 0)) {
        errno = EINVAL;
        return NULL;
    }

    self = malloc(sizeof(*self));
    node_bytes = sizeof(struct list_node) + payload_bytes;
    pool = calloc(capacity, node_bytes);
    if ((self == NULL) || (pool == NULL)) {
        free(pool);
        free(self);
        errno = ENOMEM;
        return NULL;
    }

    list_setup(self, pool, payload_bytes, capacity);

    return (LIST)self;
}

/**
 *  @details    @c list を解放する.
 *              @c list は @ref list_init の戻り値である必要がある.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void list_release(LIST list)
{
    struct list *self = (struct list *)list;

    if (self != NULL) {
        free(self->pool);
        free(self);
    }
}

/**
 *  @details    @c list を空の状態にする.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int list_clear(LIST list)
{
    struct list *self = (struct list *)list;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    list_setup(self, self->pool, self->payload_bytes, self->capacity);

    return 0;
}

/**
 *  リストの先頭にノードを追加する.
 *
 *  @param  [in,out]    self    リストオブジェクト.
 *  @param  [in,out]    node    追加するノード.
 */
void list_insert_head(struct list *self, struct list_node *node)
{
    if (self->root == NULL) {
        self->root = self->last = node;
    } else {
        node->next = self->root;
        self->root->prev = node;
        self->root = node;
    }
}

/**
 *  リストの末尾にノードを追加する.
 *
 *  @param  [in,out]    self    リストオブジェクト.
 *  @param  [in,out]    node    追加するノード.
 */
void list_insert_tail(struct list *self, struct list_node *node)
{
    if (self->last == NULL) {
        self->root = self->last = node;
    } else {
        node->prev = self->last;
        self->last->next = node;
        self->last = node;
    }
}

/**
 *  @details    @c list の指定位置に要素を追加する.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @param      [in]        index   追加する位置.
 *  @param      [in]        payload リストに追加するデータ.
 *  @return     成功時は, 追加したリスト上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @remarks    @c index が負数の場合はリストの最後に追加される.
 *  @warning    スレッドセーフではない.
 */
void *list_insert(LIST list, int index, void *payload)
{
    struct list *self = (struct list *)list;
    struct list_node *node;
    struct list_node *iter;
    int i;

    if ((self == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return NULL;
    }

    node = list_pop_released(self);
    if (node == NULL) {
        return NULL;
    }
    *node = LIST_NODE_INITIALIZER;
    memcpy(node->payload, payload, self->payload_bytes);

    if (index == 0) {
        list_insert_head(self, node);
    } else if (index > 0) {
        iter = self->root;
        for (i = 0; i < index; ++i) {
            if (iter == NULL) {
                errno = EINVAL;
                return NULL;
            }
            iter = iter->next;
        }
        if (iter == NULL) {
            node->prev = self->last;
            self->last->next = node;
            self->last = node;
        } else {
            node->next = iter;
            node->prev = iter->prev;
            node->prev->next = node;
            iter->prev = node;
        }
    } else {
        list_insert_tail(self, node);
    }
    ++self->count;

    return node->payload;
}

/**
 *  @details    @c list の末尾に要素を追加する.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @param      [in]        payload リストに追加するデータ部.
 *  @return     成功時は, 追加したリスト上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *list_add(LIST list, void *payload)
{
    return list_insert(list, -1, payload);
}

/**
 *  @details    @c list から指定のデータを削除する.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @param      [in]        iter    削除する要素の反復子.
 *  @return     成功時は 0 が返る.
 *              失敗時は -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int list_remove(LIST list, ITER iter)
{
    struct list *self = (struct list *)list;
    struct list_node *node;

    if ((self == NULL) || (iter == NULL)) {
        errno = EINVAL;
        return -1;
    }

    node = (struct list_node *)iter;
    --self->count;
    if (self->root == node) {
        self->root = node->next;
    }
    if (self->last == node) {
        self->last = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    list_push_released(self, node);

    return 0;
}

/**
 *  @details    @c list のデータ部のサイズを取得する.
 *
 *  @param      [in]    list    リストオブジェクト.
 *  @return     成功時は, @c list のデータ部のサイズが返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t list_payload_bytes(LIST list)
{
    struct list *self = (struct list *)list;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->payload_bytes;
}

/**
 *  @details    @c list に追加されている要素の数を返す.
 *
 *  @param      [in]    list    リストオブジェクト.
 *  @return     成功時は, @c list に追加されている要素の数を返す.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t list_count(LIST list)
{
    struct list *self = (struct list *)list;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->count;
}

/**
 *  @details    @c list の反復子を取得する.
 *
 *  @code
 *  for (ITER iter = list_iter(list);
 *       iter != NULL;
 *       iter = iter_next(iter)) {
 *      void *payload = iter_get_payload(iter);
 *  }
 *  @endcode
 *
 *  @param      [in]    list    リストオブジェクト.
 *  @return     成功時は, @c list の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 *  @sa         iter_next, iter_get_payload
 */
ITER list_iter(LIST list)
{
    struct list *self = (struct list *)list;
    if (self == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (ITER)self->root;
}

/**
 *  @details    @c list に積まれた要素を配列にコピーする.
 *
 *  @param      [in]    list    リストオブジェクト.
 *  @param      [out]   array   確保した配列のポインタ.
 *  @param      [out]   count   要素の数.
 *  @return     成功時は, 0 が返り, @c array に確保された配列のポインタを
 *              設定する.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */

int list_to_array(LIST list, void **array, size_t *count)
{
    struct list *self = (struct list *)list;
    ITER iter;
    void *buf;

    if ((self == NULL) || (array == NULL) || (count == NULL)) {
        errno = EINVAL;
        return -1;
    }

    buf = malloc(self->payload_bytes * self->count);
    if (buf == NULL) {
        return -1;
    }
    *array = buf;
    *count = 0;
    for (iter = list_iter(list); iter != NULL; iter = iter_next(iter)) {
        memcpy(buf, iter_get_payload(iter), self->payload_bytes);
        buf = (void *)((uintptr_t)buf + self->payload_bytes);
        ++(*count);
    }

    return 0;
}

/**
 *  @details    空で, 指定の容量を備えた, @ref STACK オブジェクトを確保
 *              および初期化する.
 *
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        スタックの容量.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
STACK stack_init(size_t payload_bytes, size_t capacity)
{
    return (STACK)list_init(payload_bytes, capacity);
}

/**
 *  @details    @c stack を解放する.
 *              @c stack は @ref stack_init の戻り値である必要がある.
 *
 *  @param      [in,out]    stack   スタックオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void stack_release(STACK stack)
{
    list_release((LIST)stack);
}

/**
 *  @details    @c stack を空の状態にする.
 *
 *  @param      [in,out]    stack   スタックオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int stack_clear(STACK stack)
{
    return list_clear((LIST)stack);
}

/**
 *  @details    @c stack に要素を積む.
 *
 *  @param      [in,out]    stack   スタックオブジェクト.
 *  @param      [in]        payload スタックに積むデータ.
 *  @return     成功時は, 積んだスタック上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *stack_push(STACK stack, void *payload)
{
    return list_insert((LIST)stack, 0, payload);
}

/**
 *  @details    @c stack から, 最初の要素を取り除く.
 *
 *  @param      [in,out]    stack   スタックオブジェクト.
 *  @param      [out]       payload データ部をコピーするバッファ.
 *  @return     成功時は, @c stack に残っている要素の数が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int stack_pop(STACK stack, void *payload)
{
    LIST list = (LIST)stack;
    ITER iter = list_iter(list);

    if ((list == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return -1;
    }
    if (iter == NULL) {
        errno = EAGAIN;
        return -1;
    }

    memcpy(payload, iter_get_payload(iter), list_payload_bytes(list));
    list_remove(list, iter);

    return list_count(list);
}

/**
 *  @details    @c stack に積まれている要素の数を返す.
 *
 *  @param      [in]    stack   スタックオブジェクト.
 *  @return     成功時は, @c stack に積まれている要素の数を返す.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t stack_count(STACK stack)
{
    return list_count((LIST)stack);
}

/**
 *  @details    @c stack の反復子を取得する.
 *
 *  @code
 *  for (ITER iter = stack_iter(stack);
 *       iter != NULL;
 *       iter = iter_next(iter)) {
 *      void *payload = iter_get_payload(iter);
 *  }
 *  @endcode
 *
 *  @param      [in]    stack   スタックオブジェクト.
 *  @return     成功時は, @c stack の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 *  @sa         iter_next, iter_get_payload
 */
ITER stack_iter(STACK stack)
{
    return list_iter((LIST)stack);
}

/**
 *  @details    空で, 指定の容量を備えた, @ref QUEUE オブジェクトを
 *              確保および初期化する.
 *
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        キューの容量.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
QUEUE queue_init(size_t payload_bytes, size_t capacity)
{
    return (QUEUE)list_init(payload_bytes, capacity);
}

/**
 *  @details    @c que を解放する.
 *              @c que は @ref queue_init の戻り値である必要がある.
 *
 *  @param      [in,out]    que キューオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void queue_release(QUEUE que)
{
    list_release((LIST)que);
}

/**
 *  @details    @c que を空の状態にする.
 *
 *  @param      [in,out]    que キューオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int queue_clear(QUEUE que)
{
    return list_clear((LIST)que);
}

/**
 *  @details    @c que の最後に要素を追加する.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @param      [in]        payload キューに追加するデータ.
 *  @return     成功時は, 追加したキュー上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *queue_enq(QUEUE que, void *payload)
{
    return list_insert((LIST)que, -1, payload);
}

/**
 *  @details    @c que の最初の要素をコピーし, 削除する.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @param      [out]       payload データ部をコピーするバッファ.
 *  @return     成功時は, @c que に残っている要素の数が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int queue_deq(QUEUE que, void *payload)
{
    LIST list = (LIST)que;
    ITER iter = list_iter(list);

    if ((list == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return -1;
    }
    if (iter == NULL) {
        errno = EAGAIN;
        return -1;
    }

    memcpy(payload, iter_get_payload(iter), list_payload_bytes(list));
    list_remove(list, iter);

    return list_count(list);
}

/**
 *  @details    @c que に積まれた要素の数を返す.
 *
 *  @param      [in]    que キューオブジェクト.
 *  @return     成功時は, @c que に積まれた要素の数を返す.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t queue_count(QUEUE que)
{
    return list_count((LIST)que);
}

/**
 *  @details    @c que の反復子を取得する.
 *
 *  @code
 *  for (ITER iter = queue_iter(que);
 *       iter != NULL;
 *       iter = iter_next(iter)) {
 *      void *payload = iter_get_payload(iter);
 *  }
 *  @endcode
 *
 *  @param      [in]    que キューオブジェクト.
 *  @return     成功時は, @c que の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 *  @sa         iter_next, iter_get_payload
 */
ITER queue_iter(QUEUE que)
{
    return list_iter((LIST)que);
}

/**
 *  @details    @c que に積まれた要素を配列にコピーする.
 *
 *  @param      [in]    que     キューオブジェクト.
 *  @param      [out]   array   確保した配列のポインタ.
 *  @param      [out]   count   要素の数.
 *  @return     成功時は, 0 が返り, @c array に確保された配列のポインタを
 *              設定する.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */

int queue_to_array(QUEUE que, void **array, size_t *count)
{
    return list_to_array((LIST)que, array, count);
}

/**
 *  @details    空で, 指定の容量を備えた, @ref SET オブジェクトを確保
 *              および初期化する.
 *
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        セットの容量.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
SET set_init(size_t payload_bytes, size_t capacity)
{
    return (SET)list_init(payload_bytes, capacity);
}

/**
 *  @details    @c set を解放する.
 *              @c set は @ref set_init の戻り値である必要がある.
 *
 *  @param      [in,out]    set セットオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void set_release(SET set)
{
    list_release((LIST)set);
}

/**
 *  @details    @c set を空の状態にする.
 *
 *  @param      [in,out]    set セットオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int set_clear(SET set)
{
    return list_clear((LIST)set);
}

/**
 *  @details    @c set に要素を追加する.
 *              指定データがすでに追加されている場合は何もしない.
 *
 *  @param      [in,out]    set     セットオブジェクト.
 *  @param      [in]        payload セットに追加するデータ.
 *  @return     成功時は, 追加したセット上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *set_add(SET set, void *payload)
{
    LIST list = (LIST)set;
    ssize_t payload_bytes;
    ITER iter;

    if ((list == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return NULL;
    }

    payload_bytes = list_payload_bytes(list);
    for (iter = list_iter(list); iter != NULL; iter = iter_next(iter)) {
        void *p = iter_get_payload(iter);
        if (memcmp(payload, p, payload_bytes) == 0) {
            return p;
        }
    }

    return list_insert(list, -1, payload);
}

/**
 *  @details    @c set に追加されている要素の数を返す.
 *
 *  @param      [in]    set セットオブジェクト.
 *  @return     成功時は, @c set に追加されている要素の数を返す.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t set_count(SET set)
{
    return list_count((LIST)set);
}

/**
 *  @details    @c set の反復子を取得する.
 *
 *  @code
 *  for (ITER iter = set_iter(set;
 *       iter != NULL;
 *       iter = iter_next(iter)) {
 *      void *payload = iter_get_payload(iter);
 *  }
 *  @endcode
 *
 *  @param      [in]    set セットオブジェクト.
 *  @return     成功時は, @c set の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 *  @sa         iter_next, iter_get_payload
 */
ITER set_iter(SET set)
{
    return list_iter((LIST)set);
}

/**
 *  N-ary ツリーノード構造体.
 */
struct tree_node {
    struct tree_node *first_child;  /**< 最初の子要素へのポインタ. */
    struct tree_node *next_sibling; /**< 次の兄弟要素へのポインタ. */
    int age;                        /**< 世代. (ツリー上での深さ) */
    char payload[];                 /**< データ部. */
};

/**
 *  N-ary ツリーノード構造体の初期化子.
 */
#define TREE_NODE_INITIALIZER \
    (struct tree_node){       \
        .first_child = NULL,  \
        .next_sibling = NULL, \
        .age = 0              \
    }

/**
 *  N-ary ツリー管理構造体.
 */
struct tree {
    void *pool;                 /**< ツリーで使用するメモリプール. */
    struct tree_node *released; /**< 解放済みのノードのリスト. */
    struct tree_node *root;     /**< ツリーの根. */
    size_t payload_bytes;       /**< データ部のサイズ. */
    size_t capacity;            /**< 確保したノードの数. */
    size_t count;               /**< 使用中のノードの数. */
};

/**
 *  N-ary ツリー管理構造体の初期化子.
 */
#define TREE_INITIALIZER(p, b, c) \
    (struct tree){                \
        .pool = (p),              \
        .released = NULL,         \
        .root = NULL,             \
        .payload_bytes = (b),     \
        .capacity = (c),          \
        .count = 0                \
    }

/**
 *  N-ary ツリー反復子構造体.
 */
struct tree_iter {
    STACK fringe;               /**< 探索経路を保持するスタック. */
    int age;                    /**< 現在地の世代. */
    void *payload;              /**< 現在地のデータ部. */
};

/**
 *  N-ary ツリー反復子構造体の初期化子.
 */
#define TREE_ITER_INITIALIZER \
    (struct tree_iter){       \
        .fringe = NULL,       \
        .age = 0,             \
        .payload = NULL       \
    }

/**
 *  N-ary ツリー向け, 解放済みノードのリストにノードを追加する.
 *
 *  @param  [in,out]    self    ツリーオブジェクト.
 *  @param  [in]        node    追加するノード.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 *  @pre    @c node の非 NULL は呼び出し側で保証すること.
 */
static inline void tree_push_released(struct tree *self, struct tree_node *node)
{
    node->first_child = NULL;
    if (self->released == NULL) {
        node->next_sibling = NULL;
    } else {
        node->next_sibling = self->released;
    }
    self->released = node;
}

/**
 *  N-ary ツリー向け, 解放済みノードのリストからノードを取得する.
 *
 *  @param  [in,out]    self    ツリーオブジェクト.
 *  @return 成功時は, ノードのポインタが返る.
 *          失敗時は, NULL が返り, errno が適切に設定される.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 */
static inline struct tree_node *tree_pop_released(struct tree *self)
{
    struct tree_node *node = self->released;

    if (node == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    self->released = node->next_sibling;

    return node;
}

/**
 *  ツリーの初期設定を行う.
 *
 *  @param  [in,out]    self            ツリーオブジェクト.
 *  @param  [in]        pool            ツリーに使用するメモリプール.
 *  @param  [in]        payload_bytes   データ部のサイズ.
 *  @param  [in]        capacity        リストの容量.
 *  @pre    @c self の非 NULL は呼び出し側で保証すること.
 *  @pre    @c pool の非 NULL は呼び出し側で保証すること.
 */
static inline void tree_setup(struct tree *self,
                              void *pool,
                              size_t payload_bytes,
                              size_t capacity)
{
    size_t node_bytes;
    struct tree_node *node;

    *self = TREE_INITIALIZER(pool, payload_bytes, capacity);
    node_bytes = sizeof(*node) + self->payload_bytes;
    for (size_t i = 0; i < self->capacity + 1; ++i) {
        node = (struct tree_node *)((uintptr_t)self->pool + (node_bytes * i));
        tree_push_released(self, node);
    }

    /* root は固定で割り当てる. */
    /** @todo root は他の要素と重複しなさそうなデータ部にする,
     *  もしくは区別できる仕組みを入れる.
     */
    self->root = tree_pop_released(self);
    *(self->root) = TREE_NODE_INITIALIZER;
    memset(self->root->payload, 0x5A, self->payload_bytes);
}

/**
 *  @details    空で, 指定の容量を備えた, @ref TREE オブジェクトを確保
 *              および初期化する.
 *              N-ary ツリーのため, 同一の親に複数の子を追加することができる.
 *
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        ツリーの容量.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
TREE tree_init(size_t payload_bytes, size_t capacity)
{
    struct tree *self;
    size_t node_bytes;
    void *pool;

    if ((payload_bytes == 0) || (capacity == 0)) {
        errno = EINVAL;
        return NULL;
    }

    self = malloc(sizeof(*self));
    node_bytes = sizeof(struct tree_node) + payload_bytes;
    pool = calloc(capacity + 1, node_bytes);
    if ((self == NULL) || (pool == NULL)) {
        free(pool);
        free(self);
        errno = ENOMEM;
        return NULL;
    }

    tree_setup(self, pool, payload_bytes, capacity);

    return (TREE)self;
}

/**
 *  @details    @c tree を解放する.
 *              @c tree は @ref tree_init の戻り値である必要がある.
 *
 *  @param      [in,out]    tree    ツリーオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void tree_release(TREE tree)
{
    struct tree *self = (struct tree *)tree;

    if (self != NULL) {
        free(self->pool);
        free(self);
    }
}

/**
 *  @details    @c tree を空の状態にする.
 *
 *  @param      [in,out]    tree    ツリーオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int tree_clear(TREE tree)
{
    struct tree *self = (struct tree *)tree;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    tree_setup(self, self->pool, self->payload_bytes, self->capacity);

    return 0;
}

/**
 *  @details    @c t に追加された @c parent の子に要素を追加するための再帰処理.
 *              @c parent が複数存在する場合は, 最初に見つかった要素の子として
 *              追加する.
 *
 *  @param      [in,out]    self    ツリーオブジェクト.
 *  @param      [in,out]    node    探索中の要素.
 *  @param      [in]        age     探索中の世代 (深さ).
 *  @param      [in]        parent  探索対象のデータ部.
 *  @param      [in]        payload ツリーに追加するデータ.
 *  @return     成功時は, 追加したツリー上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @remarks    探索は幅 (同一の親に追加された要素) 優先で行われる.
 *  @pre        @c self の非 NULL は呼び出し側で保証すること.
 *  @pre        @c node の非 NULL は呼び出し側で保証すること.
 *  @pre        @c parent の非 NULL は呼び出し側で保証すること.
 *  @pre        @c payload の非 NULL は呼び出し側で保証すること.
 */
static void *tree_insert_inner(struct tree *self,
                               struct tree_node *node,
                               int age,
                               void *parent,
                               void *payload)
{
    void *additional = NULL;

    if (memcmp(node->payload, parent, self->payload_bytes) == 0) {
        struct tree_node *child = node->first_child;
        if (child == NULL) {
            node->first_child = tree_pop_released(self);
            if (node->first_child == NULL) {
                return NULL;
            }
            *node->first_child = TREE_NODE_INITIALIZER;
            node->first_child->age = age;
            memcpy(node->first_child->payload, payload, self->payload_bytes);
            additional = node->first_child->payload;
        } else {
            while (child->next_sibling != NULL) {
                child = child->next_sibling;
            }
            child->next_sibling = tree_pop_released(self);
            if (child->next_sibling == NULL) {
                return NULL;
            }
            *child->next_sibling = TREE_NODE_INITIALIZER;
            child->next_sibling->age = age;
            memcpy(child->next_sibling->payload, payload, self->payload_bytes);
            additional = child->next_sibling->payload;
        }
        ++self->count;
    } else {
        if (node->next_sibling != NULL) {
            additional = tree_insert_inner(self,
                                           node->next_sibling,
                                           age,
                                           parent,
                                           payload);
        }
        if ((additional == NULL) && (node->first_child != NULL)) {
            additional = tree_insert_inner(self,
                                           node->first_child,
                                           age + 1,
                                           parent,
                                           payload);
        }
    }

    return additional;
}

/**
 *  @details    @c tree の指定位置に要素を追加する.
 *
 *  @param      [in,out]    tree    ツリーオブジェクト.
 *  @param      [in]        parent  子として追加する親要素.
 *  @param      [in]        payload ツリーに追加するデータ.
 *  @return     成功時は, 追加したツリー上のデータ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *tree_insert(TREE tree, void *parent, void *payload)
{
    struct tree *self = (struct tree *)tree;

    if ((self == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return NULL;
    }

    return tree_insert_inner(self,
                             self->root,
                             1,
                             (parent ?: self->root->payload),
                             payload);
}

/**
 *  @details    @c tree に追加されている要素の数を返す.
 *
 *  @param      [in]    tree    ツリーオブジェクト.
 *  @return     成功時は, @c tree に追加されている要素の数を返す.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
ssize_t tree_count(TREE tree)
{
    struct tree *self = (struct tree *)tree;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->count;
}

/**
 *  @details    @c tree の反復子を取得する.
 *
 *  @code
 *  for (TREE_ITER iter = tree_iter_get(tree);
 *       iter != NULL;
 *       iter = tree_iter_next(iter)) {
 *      void *payload = tree_iter_get_payload(iter);
        int age = tree_iter_get_age(iter);
 *  }
 *  @endcode
 *
 *  @param      [in]    tree    ツリーオブジェクト.
 *  @return     成功時は, @c tree の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @remarks    走査は深さ (子要素) 優先で行われる.
 *  @warning    スレッドセーフではない.
 *  @sa         tree_iter_next, tree_iter_get_payload, tree_iter_get_age
 */
TREE_ITER tree_iter_get(TREE tree)
{
    struct tree *self = (struct tree *)tree;
    struct tree_iter *iter;

    iter = malloc(sizeof(*iter));
    if (iter == NULL) {
        return NULL;
    }
    *iter = TREE_ITER_INITIALIZER;
    iter->fringe = stack_init(sizeof(struct tree_node *), self->capacity);
    if (iter->fringe == NULL) {
        free(iter);
        return NULL;
    }

    if (self->root->first_child != NULL) {
        stack_push(iter->fringe, &self->root->first_child);
    }

    if (tree_iter_next((TREE_ITER)iter) == NULL) {
        tree_iter_release((TREE_ITER)iter);
        iter = NULL;
    }
    return (TREE_ITER)iter;
}

/**
 *  @details    @c iter を解放する.
 *              @c iter は @ref tree_iter_get の戻り値である必要がある.
 *
 *  @param      [in,out]    iter    反復子オブジェクト.
 *  @warning    スレッドセーフではない.
 */
void tree_iter_release(TREE_ITER iter)
{
    struct tree_iter *self = (struct tree_iter *)iter;

    if (self != NULL) {
        stack_release(self->fringe);
        free(self);
    }
}

/**
 *  @details    @c iter の次の反復子を取得する.
 *
 *  @param      [in]    iter    N-ary ツリーの反復子.
 *  @return     成功時は, 次の反復子が返る. 次の要素がない場合は NULL が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
TREE_ITER tree_iter_next(TREE_ITER iter)
{
    struct tree_iter *self = (struct tree_iter *)iter;
    struct tree_node *node;
    int ret;

    if (self == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (stack_count(self->fringe) == 0) {
        errno = ENOENT;
        return NULL;
    }

    ret = stack_pop(self->fringe, &node);
    if (ret == -1) {
        return NULL;
    }
    self->age = node->age;
    self->payload = node->payload;
    if (node->next_sibling != NULL) {
        stack_push(self->fringe, &node->next_sibling);
    }
    if (node->first_child != NULL) {
        stack_push(self->fringe, &node->first_child);
    }

    return iter;
}

/**
 *  @details    @c iter のデータ部を取得する.
 *
 *  @param      [in]    iter    N-ary ツリーの反復子.
 *  @return     成功時は, データ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *tree_iter_get_payload(TREE_ITER iter)
{
    struct tree_iter *self = (struct tree_iter *)iter;

    if (self == NULL) {
        errno = EINVAL;
        return NULL;
    }

    return self->payload;
}

/**
 *  @details    @c iter のツリー上での世代 (深さ) を取得する.
 *
 *  @param      [in]    iter    N-ary ツリーの反復子.
 *  @return     成功時は, 世代が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
int tree_iter_get_age(TREE_ITER iter)
{
    struct tree_iter *self = (struct tree_iter *)iter;

    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->age;
}
