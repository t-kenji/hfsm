/** @file   collections.c
 *  @brief  コレクションに関する機能を提供する.
 *
 *  コレクション (リスト, スタック, キュー) を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
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
 *  @param [in,out]    l    リストオブジェクト.
 *  @param [in]        node 追加するノード.
 */
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

/**
 *  解放済みノードのリストからノードを取得する.
 *
 *  @param  [in,out]    l   リストオブジェクト.
 *  @return 成功時は, ノードのポインタが返る.
 *          失敗時は, NULL が返り, errno が適切に設定される.
 */
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

/**
 *  リストの初期設定を行う.
 *
 *  @param  [in,out]    l               リストオブジェクト.
 *  @param  [in]        pool            リストに使用するメモリプール.
 *  @param  [in]        payload_bytes   データ部のサイズ.
 *  @param  [in]        capacity        リストの容量.
 */
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

/**
 *  @details    @c list のオブジェクトを解放する.
 *              @c list は @ref list_init の戻り値である必要がある.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @warning    スレッドセーフではない.
 */
void list_release(LIST list)
{
    struct list *l = (struct list *)list;

    if (l != NULL) {
        free(l->pool);
        free(l);
    }
}

/**
 *  @details    @c list のオブジェクトを空の状態にする.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
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

/**
 *  @details    @c list の指定位置に要素を追加する.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @param      [in]        index   追加する位置.
 *  @param      [in]        payload リストに追加するデータ.
 *  @return     成功時は, 追加したリスト上のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @remarks    @c index が負数の場合はリストの最後に追加される.
 *  @warning    スレッドセーフではない.
 */
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

/**
 *  @details    @c list の末尾に要素を追加する.
 *
 *  @param      [in,out]    list    リストオブジェクト.
 *  @param      [in]        payload リストに追加するデータ部.
 *  @return     成功時は, 追加したリスト上のポインタが返る.
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

/**
 *  @details    @c list の反復子を取得する.
 *
 *  @param      [in]    list    リストオブジェクト.
 *  @return     成功時は, @c list の反復子が返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
LIST_ITER list_iter(LIST list)
{
    struct list *l = (struct list *)list;
    if (l == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (LIST_ITER)l->root;
}

/**
 *  @details    @c iter の次の反復子を取得する.
 *
 *  @param      [in]    iter    リストの反復子.
 *  @return     成功時は, 次の反復子が返り.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
LIST_ITER list_next(LIST_ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return (LIST_ITER)((struct list_node *)iter)->next;
}

/**
 *  @details    @c iter のデータ部を取得する.
 *
 *  @param      [in]    iter    リストの反復子.
 *  @return     成功時は, データ部のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 *  @warning    スレッドセーフではない.
 */
void *list_get_payload(LIST_ITER iter)
{
    if (iter == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return ((struct list_node *)iter)->payload;
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
 *  @details    @c stack のオブジェクトを解放する.
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
 *  @details    @c stack のオブジェクトを空の状態にする.
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
 *  @return     成功時は, 積んだスタック上のポインタが返る.
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
    struct list *l = (struct list *)stack;
    LIST_ITER iter = list_iter((LIST)l);

    if ((l == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return -1;
    }
    if (iter == NULL) {
        errno = EAGAIN;
        return -1;
    }

    memcpy(payload, list_get_payload(iter), l->payload_bytes);
    list_remove((LIST)l, iter);

    return l->count;
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
 *  @details    @c que のオブジェクトを解放する.
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
 *  @details    @c que のオブジェクトを空の状態にする.
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
 *  @return     成功時は, 追加したキュー上のポインタが返る.
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
    struct list *l = (struct list *)que;
    LIST_ITER iter = list_iter((LIST)l);

    if ((l == NULL) || (payload == NULL)) {
        errno = EINVAL;
        return -1;
    }
    if (iter == NULL) {
        errno = EAGAIN;
        return -1;
    }

    memcpy(payload, list_get_payload(iter), l->payload_bytes);
    list_remove((LIST)l, iter);

    return l->count;
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
    struct list *l = (struct list *)que;

    if (l == NULL) {
        errno = EINVAL;
        return -1;
    }

    return l->count;
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
    struct list *l = (struct list *)que;
    LIST_ITER iter;
    void *buf;

    if ((l == NULL) || (array == NULL) || (count == 0)) {
        errno = EINVAL;
        return -1;
    }

    *count = 0;
    *array = buf = malloc(l->payload_bytes * l->count);
    for (iter = list_iter((LIST)l); iter != NULL; iter = list_next(iter)) {
        memcpy(buf, list_get_payload(iter), l->payload_bytes);
        buf += l->payload_bytes;
        ++(*count);
    }

    return 0;
}
