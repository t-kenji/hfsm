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
#ifndef __HFSM_COLLECTIONS_H__
#define __HFSM_COLLECTIONS_H__

#include <unistd.h>

/** @defgroup cat_collections Collections
 *  汎用コレクションを提供するモジュール.
 */

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

/** @addtogroup cat_list List 構造
 *  List 構造を提供するモジュール.
 *  @ingroup cat_collections
 *  @{
 */

/**
 *  汎用リスト型.
 */
typedef struct {} *LIST;

/**
 *  リストオブジェクトを初期化する.
 *
 *  @par    使用例
 *          @code
 *          LIST list = list_init(sizeof(int), 100);
 *          int data;
 *          data = 1;
 *          list_add(list, &data);
 *          data = 2;
 *          list_add(list, &data);
 *          for (ITER iter = list_iter(list); iter != NULL; iter = iter_next(iter)) {
 *              data = *(int *)iter_get_payload(iter);
 *              // do something.
 *          }
 *          list_release(list);
 *          @endcode
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
 *  リストのデータ部のサイズを取得する.
 */
ssize_t list_payload_bytes(LIST list);

/**
 *  リストの長さを取得する.
 */
ssize_t list_count(LIST list);

/**
 *  リストの反復子を取得する.
 */
ITER list_iter(LIST list);

/**
 *  リストを配列に変換する.
 */
int list_to_array(LIST list, void **array, size_t *count);

/** @} */

/** @addtogroup cat_stack Stack 構造
 *  Stack 構造を提供するモジュール.
 *  @ingroup cat_collections
 *  @{
 */

/**
 *  汎用スタック型.
 */
typedef struct {} *STACK;

/**
 *  スタックオブジェクトを初期化する.
 *
 *  @par    使用例
 *          @code
 *          STACK stack = stack_init(sizeof(int), 100);
 *          int data;
 *          data = 1;
 *          stack_push(stack, &data);
 *          data = 2;
 *          stack_push(stack, &data);
 *          stack_pop(stack, &data);
 *          // do something.
 *          stack_pop(stack, &data);
 *          // do something.
 *          stack_release(stack);
 *          @endcode
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

/** @} */

/** @addtogroup cat_queue Queue 構造
 *  Queue 構造を提供するモジュール.
 *  @ingroup cat_collections
 *  @{
 */

/**
 *  汎用キュー型.
 */
typedef struct {} *QUEUE;

/**
 *  キューオブジェクトを初期化する.
 *
 *  @par    使用例
 *          @code
 *          QUEUE que = queue_init(sizeof(int), 100);
 *          int data;
 *          data = 1;
 *          queue_enq(que, &data);
 *          data = 2;
 *          queue_enq(que, &data);
 *          queue_deq(que, &data);
 *          // do something.
 *          queue_deq(que, &data);
 *          // do something.
 *          queue_release(que);
 *          @endcode
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
 *  キューの反復子を取得する.
 */
ITER queue_iter(QUEUE que);

/**
 *  キューを配列に変換する.
 */
int queue_to_array(QUEUE que, void **array, size_t *count);

/** @} */

/** @addtogroup cat_set Set 構造
 *  Set 構造を提供するモジュール.
 *  @ingroup cat_collections
 *  @{
 */

/**
 *  汎用セット型.
 */
typedef struct {} *SET;

/**
 *  セットオブジェクトを初期化する.
 *
 *  @par    使用例
 *          @code
 *          SET set = set_init(sizeof(int), 100);
 *          int data;
 *          data = 1;
 *          set_add(set, &data);
 *          data = 2;
 *          set_add(set, &data);
 *          for (ITER iter = set_iter(set); iter != NULL; iter = iter_next(iter)) {
 *              data = *(int *)iter_get_payload(iter);
 *              // do something.
 *          }
 *          set_release(set);
 *          @endcode
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

/** @} */

/** @addtogroup cat_tree N-ary Tree 構造
 *  N-ary Tree 構造を提供するモジュール.
 *  @ingroup cat_collections
 *  @{
 */

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
 *
 *  @par    使用例
 *          @code
 *          TREE tree = tree_init(sizeof(int), 100);
 *          int data, parent;
 *          data = 1;
 *          tree_insert(tree, NULL, &data);
 *          data = 2;
 *          tree_insert(tree, NULL, &data);
 *          data = 11;
 *          parent = 1;
 *          tree_insert(tree, &parent, &data);
 *          data = 21;
 *          parent = 2;
 *          tree_insert(tree, &parent, &data);
 *          for (TREE_ITER iter = tree_iter_get(tree);
 *               iter != NULL;
 *               iter = tree_iter_next(iter)) {
 *              data = *(int *)tree_iter_get_payload(iter);
 *              // do something.
 *          }
 *          tree_release(tree);
 *          @endcode
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

/** @} */

#endif /* __HFSM_COLLECTIONS_H__ */
