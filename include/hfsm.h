/** @file   hfsm.h
 *  @brief  階層型有限状態マシン. (HFSM: Hierarchical Finite State Machine)
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-19 新規作成.
 */
#ifndef __HFSM_HFSM_H__
#define __HFSM_HFSM_H__

#include <stdbool.h>

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
    const char *name;                      /**< 状態名. (デバッグ用途) */
    const struct fsm_state * const parent; /**< 親状態. */
    struct fsm_state_variable *variable;   /**< 状態変数. */

                                           /**  entry アクション. */
    void (* const entry)(struct fsm *machine, void *data, bool cmpl);
                                           /**  do アクティビティ. */
    void (* const exec)(struct fsm *machine, void *data);
                                           /**  exit アクション. */
    void (* const exit)(struct fsm *machine, void *data, bool cmpl);
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
#define FSM_STATE(var, prnt, dat, ent, exe, exi)   \
    static struct fsm_state_variable var##_var = { \
        .history = NULL,                           \
        .data = (dat)                              \
    };                                             \
    static const struct fsm_state var =            \
        FSM_STATE_HELPER(#var, (prnt), &var##_var, (ent), (exe), (exi))

/**
 *  開始状態.
 *
 *  @ref struct hfsm の初期状態となる.
 */
extern const struct fsm_state state_start;

/**
 *  遷移構造体.
 */
struct fsm_trans {
    const struct fsm_state *from; /**< 起点となる状態. */
    int event;                    /**< 関連付けるイベント. */
    void (*action)(struct fsm *); /**< イベントアクション */
    const struct fsm_state *to;   /**< 遷移先の状態 */
};

/**
 *  遷移構造体の設定ヘルパ.
 */
#define FSM_TRANS_HELPER(f, e, a, t) \
    {                                \
        .from = (f),                 \
        .event = (e),                \
        .action = (a),               \
        .to = (t)                    \
    }

/**
 *  遷移構造体の初期化子.
 */
#define FSM_TRANS_INITIALIZER \
    (struct fsm_trans)FSM_TRANS_HELPER(NULL, -1, NULL, NULL)

/**
 *  遷移対応配列の終端.
 */
#define FSM_TRANS_TERMINATOR FSM_TRANS_INITIALIZER

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
void fsm_transition(struct fsm *machine, int event);

/**
 *  現在の状態の do アクティビティを実行する.
 */
void fsm_update(struct fsm *machine);

/**
 *  現在の状態名を取得する.
 */
void fsm_current_state(struct fsm *machine, char *name, size_t len);

#endif /* __HFSM_HFSM_H__ */
