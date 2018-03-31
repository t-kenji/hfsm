/** @file   hfsm.h
 *  @brief  階層型有限状態マシン. (HFSM: Hierarchical Finite State Machine)
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-19 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
#ifndef __HFSM_HFSM_H__
#define __HFSM_HFSM_H__

#include <stdbool.h>
#include <string.h>

#include "collections.h"

struct fsm_trans;
struct fsm;

/**
 *  状態変数構造体.
 */
struct fsm_state_variable {
    const struct fsm_state *history; /**< 履歴状態. */
    void *data;                      /**< 状態固有情報. */
};

/**
 *  状態変数構造体の初期化子.
 */
#define FSM_STATE_VARIABLE_INITIALIZER \
    (struct fsm_state_variable){       \
        .history = NULL,               \
        .data = NULL                   \
    }

/**
 *  状態構造体.
 */
struct fsm_state {
    const char *name;                                 /**< 状態名. */
    const struct fsm_state * const parent;            /**< 親状態. */
    struct fsm_state_variable *variable;              /**< 状態変数. */

    void (* const entry)(struct fsm *, void *, bool); /**  entry アクション. */
    void (* const exec)(struct fsm *, void *);        /**  do アクティビティ. */
    void (* const exit)(struct fsm *, void *, bool);  /**  exit アクション. */
};

/**
 *  状態構造体設定ヘルパ.
 */
#define FSM_STATE_HELPER(nam, prnt, var, ent, exe, exi) \
    {                                                   \
        .name = (nam),                                  \
        .parent = (prnt),                               \
        .variable = (var),                              \
        .entry = (ent),                                 \
        .exec = (exe),                                  \
        .exit = (exi)                                   \
    }

/**
 *  状態構造体初期化子.
 */
#define FSM_STATE_INITIALIZER(nam) \
    (struct fsm_state)FSM_STATE_HELPER((nam), NULL, NULL, NULL, NULL, NULL)

/**
 *  状態定義ヘルパ.
 */
#define FSM_STATE(var, prnt, dat, ent, exe, exi)                         \
    static struct fsm_state_variable var##_var = {                       \
        .history = NULL,                                                 \
        .data = (dat)                                                    \
    };                                                                   \
    static const struct fsm_state var##_ =                               \
        FSM_STATE_HELPER(#var, (prnt), &var##_var, (ent), (exe), (exi)), \
                                  *var = &var##_

/**
 *  開始状態.
 */
extern const struct fsm_state *state_start;

/**
 *  終了状態.
 */
extern const struct fsm_state *state_end;


/**
 *  イベント構造体.
 */
struct fsm_event {
    const char *name; /**< イベント名. */
};

/**
 *  イベント構造体設定ヘルパ.
 */
#define FSM_EVENT_HELPER(nam) \
    {                         \
        .name = (nam)         \
    }

/**
 *  イベント構造体初期化子.
 */
#define FSM_EVENT_INITIALIZER(nam) (struct fsm_event)FSM_EVENT_HELPER(nam)

/**
 *  イベント定義ヘルパ.
 */
#define FSM_EVENT(var) \
    static const struct fsm_event var##_ = FSM_EVENT_HELPER(#var), \
			          *var = &var##_

/**
 *  Null 遷移イベント.
 *
 *  @ref fsm_transition 後に必ず発生する.
 */
extern const struct fsm_event *event_null;

/**
 *  ガード条件構造体.
 */
struct fsm_cond {
    const char *name;                 /**< 条件名. */
    bool (*const func)(struct fsm *); /**< ガード条件. */
};

/**
 *  ガード条件構造体設定ヘルパ.
 */
#define FSM_COND_HELPER(nam, fn) \
    {                            \
        .name = (nam),           \
        .func = (fn)             \
    }

/**
 *  ガード条件定義ヘルパ.
 */
#define FSM_COND(var, args)                      \
    static bool var##_func args;                 \
    static const struct fsm_cond var##_ =        \
        FSM_COND_HELPER(#var, var##_func),       \
                                 *var = &var##_; \
    static bool var##_func args

/**
 *  遷移アクション構造体.
 */
struct fsm_action {
    const char *name;                 /**< アクション名. */
    void (*const func)(struct fsm *); /**< 遷移アクション. */
};

/**
 *  遷移アクション構造体設定ヘルパ.
 */
#define FSM_ACTION_HELPER(nam, fn) \
    {                              \
        .name = (nam),             \
        .func = (fn)               \
    }

/**
 *  遷移アクション定義ヘルパ.
 */
#define FSM_ACTION(var, args)                      \
    static void var##_func args;                   \
    static const struct fsm_action var##_ =        \
        FSM_ACTION_HELPER(#var, var##_func),       \
                                   *var = &var##_; \
    static void var##_func args

/**
 *  遷移構造体.
 */
struct fsm_trans {
    const struct fsm_state *from;    /**< 起点となる状態. */
    const struct fsm_event *event;   /**< 関連付けるイベント. */
    const struct fsm_cond *cond;     /**< ガード条件. */
    const struct fsm_action *action; /**< イベントアクション */
    const struct fsm_state *to;      /**< 遷移先の状態 */
};

/**
 *  遷移構造体の設定ヘルパ.
 */
#define FSM_TRANS_HELPER(f, e, c, a, t) \
    {                                   \
        .from = (f),                    \
        .event = (e),                   \
        .cond = (c),                    \
        .action = (a),                  \
        .to = (t)                       \
    }

/**
 *  遷移構造体の初期化子.
 */
#define FSM_TRANS_INITIALIZER \
    (struct fsm_trans)FSM_TRANS_HELPER(NULL, NULL, NULL, NULL, NULL)

/**
 *  遷移対応配列の終端.
 */
#define FSM_TRANS_TERMINATOR FSM_TRANS_INITIALIZER

/**
 *  ダンプ処理の標準ハンドラ.
 */
static inline void fsm_dump_state_transition_to_text(TREE tree)
{
    TREE_ITER iter;

    for (iter = tree_iter_get(tree);
         iter != NULL;
         iter = tree_iter_next(iter)) {

        const struct fsm_state *state =
            *(const struct fsm_state **)tree_iter_get_payload(iter);
        int age = tree_iter_get_age(iter);
        char spacer[128] = {'\0'};
        for (int i = 0; i < age; ++i) {
            strncat(spacer, "    ", sizeof(spacer) - (i * 4));
        }
        printf("%s%s\n", spacer, state->name);
    }

    tree_iter_release(iter);
}

/**
 *  状態マシンを初期化する.
 */
struct fsm *fsm_init(const struct fsm_trans *corresps);

/**
 *  状態マシンを破棄する.
 */
int fsm_term(struct fsm *machine);

/**
 *  イベントによる状態遷移を実施する.
 */
void fsm_transition(struct fsm *machine, const struct fsm_event *event);

/**
 *  現在の状態の do アクティビティを実行する.
 */
void fsm_update(struct fsm *machine);

/**
 *  状態の固有情報を取得する.
 */
void *fsm_get_state_data(const struct fsm_state *state);

/**
 *  現在の状態名を取得する.
 */
void fsm_current_state(struct fsm *machine, char *name, size_t len);

/**
 *  状態遷移設定をダンプする.
 */
void fsm_dump_state_transition(struct fsm *machine, void (*handler)(TREE));

#endif /* __HFSM_HFSM_H__ */
