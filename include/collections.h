/** @file   collections.h
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
#ifndef __ANTTQ_COLLECTIONS_H__
#define __ANTTQ_COLLECTIONS_H__

#include <stdlib.h>

/**
 *  汎用反復子型
 */
typedef struct {} *ITER;

/**
 *  反復子の次を取得する.
 */
ITER iter_next(ITER iter);

/**
 *  反復子のデータ部を取得する.
 */
void *iter_get_payload(ITER iter);

/**
 *  汎用リスト型.
 */
typedef struct {} *LIST;

/**
 *  リストオブジェクトを初期化する.
 */
LIST list_init(size_t payload_bytes, size_t capacity);

/**
 *  リストオブジェクトを解放する.
 */
void list_release(LIST list);

/**
 *  リスト要素をすべて消去する.
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
int list_remove(LIST list, ITER iter);

/**
 *  リストの長さを取得する.
 */
ssize_t list_count(LIST list);

/**
 *  リストの反復子を取得する.
 */
ITER list_iter(LIST list);

/**
 *  汎用スタック型.
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
 *  スタック要素をすべて消去する.
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
 *  スタックの深さを取得する.
 */
ssize_t stack_count(STACK stack);

/**
 *  スタックの反復子を取得する.
 */
ITER stack_iter(STACK stack);

/**
 *  汎用キュー型.
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
 *  キュー要素をすべて消去する.
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

/**
 *  汎用セット型.
 */
typedef struct {} *SET;

/**
 *  セットオブジェクトを初期化する.
 */
SET set_init(size_t payload_bytes, size_t capacity);

/**
 *  セットオブジェクトを解放する.
 */
void set_release(SET set);

/**
 *  セット要素をすべて消去する.
 */
int set_clear(SET set);

/**
 *  要素をセットに追加する.
 */
void *set_add(SET set, void *payload);

/**
 *  セットの長さを取得する.
 */
ssize_t set_count(SET set);

/**
 *  セットの反復子を取得する.
 */
ITER set_iter(SET set);

/**
 *  汎用 N-ary ツリー型.
 */
typedef struct {} *TREE;

/**
 *  汎用 N-ary ツリー反復子.
 */
typedef struct {} *TREE_ITER;

/**
 *  N-ary ツリーオブジェクトを初期化する.
 */
TREE tree_init(size_t payload_bytes, size_t capacity);

/**
 *  N-ary ツリーオブジェクトを解放する.
 */
void tree_release(TREE tree);

/**
 *  N-ary ツリー要素をすべて消去する.
 */
int tree_clear(TREE tree);

/**
 *  N-ary 要素をツリーに挿入する.
 */
void *tree_insert(TREE tree, void *parent, void *payload);

/**
 *  N-ary ツリーの要素の数を取得する.
 */
ssize_t tree_count(TREE tree);

/**
 *  N-ary ツリーの反復子を取得する.
 */
TREE_ITER tree_iter_get(TREE tree);

/**
 *  N-ary ツリーの反復子を解放する.
 */
void tree_iter_release(TREE_ITER iter);

/**
 *  反復子の次を取得する.
 */
TREE_ITER tree_iter_next(TREE_ITER iter);

/**
 *  反復子のデータ部を取得する.
 */
void *tree_iter_get_payload(TREE_ITER iter);

/**
 *  反復子の N-ary ツリーでのネストの位置を取得する.
 */
int tree_iter_get_age(TREE_ITER iter);

#endif /* __ANTTQ_COLLECTIONS_H__ */
