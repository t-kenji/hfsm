/** @file   collections.h
 *  @brief  コレクションに関する機能を提供する.
 *
 *  コレクション (リスト, スタック, キュー) を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 */
#ifndef __ANTTQ_COLLECTIONS_H__
#define __ANTTQ_COLLECTIONS_H__

#include <stdlib.h>

/**
 *  リスト型.
 */
typedef struct {} *LIST;

/**
 *  リスト反復子型
 */
typedef struct {} *LIST_ITER;

/**
 *  リストオブジェクトを初期化する.
 */
LIST list_init(size_t payload_bytes, size_t capacity);

/**
 *  リストオブジェクトを解放する.
 */
void list_release(LIST list);

/**
 *  リスト要素を消去する.
 */
int list_clear(LIST list);

/**
 *  要素をリストに追加する.
 */
void *list_add(LIST list, void *payload);

/**
 *  要素をリストに挿入する.
 */
void *list_insert(LIST list, int index, void *payload);

/**
 *  要素をリストから削除する.
 */
int list_remove(LIST list, LIST_ITER iter);

/**
 *  リストの反復子を取得する.
 */
LIST_ITER list_iter(LIST list);

/**
 *  反復子の次を取得する.
 */
LIST_ITER list_next(LIST_ITER iter);

/**
 *  反復子のデータ部を取得する.
 */
void *list_get_payload(LIST_ITER iter);

/**
 *  スタック型.
 */
typedef struct {} *STACK;

/**
 *  スタックオブジェクトを初期化する.
 */
STACK stack_init(size_t payload_bytes, size_t capacity);

/**
 *  スタックオブジェクトを解放する.
 */
void stack_release(STACK stack);

/**
 *  スタック要素を消去する.
 */
int stack_clear(STACK stack);

/**
 *  要素をスタックに積む.
 */
void *stack_push(STACK stack, void *payload);

/**
 *  スタックから要素を取り出す.
 */
int stack_pop(STACK stack, void *payload);

/**
 *  キュー型.
 */
typedef struct {} *QUEUE;

/**
 *  キューオブジェクトを初期化する.
 */
QUEUE queue_init(size_t payload_bytes, size_t capacity);

/**
 *  キューオブジェクトを解放する.
 */
void queue_release(QUEUE que);

/**
 *  キュー要素を消去する.
 */
int queue_clear(QUEUE que);

/**
 *  要素をキューに積める.
 */
void *queue_enq(QUEUE que, void *payload);

/**
 *  キューから要素を取り出す.
 */
int queue_deq(QUEUE que, void *payload);

/**
 *  キューの長さを取得する.
 */
ssize_t queue_count(QUEUE que);

/**
 *  キューを配列に変換する.
 */
int queue_to_array(QUEUE que, void **array, size_t *count);

#endif /* __ANTTQ_COLLECTIONS_H__ */
