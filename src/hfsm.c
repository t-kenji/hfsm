/** @file   hfsm.c
 *  @brief  階層型有限状態マシン (HFSM: Hierarchical Finite State Machine) 実装.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "hfsm.h"

/**
 *  最大のコンポジット状態ネスト.
 */
#define NEST_MAX (5)

/**
 *  状態マシン構造体.
 */
struct fsm {
    const struct fsm_state *current;  /**< 現在の状態. */
    const struct fsm_trans *corresps; /**< 遷移の対応情報.*/

    STACK src_ancestors;              /**< 元状態の祖先を保持するバッファ. */
    STACK dest_ancestors;             /**< 先状態の祖先を保持するバッファ. */
};

/**
 *  状態マシン構造体の設定ヘルパ.
 */
#define FSM_HELPER(curr, corr, s, d) \
    (struct fsm){                    \
        .current = (curr),           \
        .corresps = (corr),          \
        .src_ancestors = (s),        \
        .dest_ancestors = (d)        \
    }

/**
 *  開始状態.
 */
const struct fsm_state state_start = FSM_STATE_INITIALIZER("start");

/**
 *  終了状態.
 */
static const struct fsm_state state_end = FSM_STATE_INITIALIZER("end");

/**
 *  指定する状態の変数を取得する.
 *
 *  呼び出し側の処理をシンプルにするため, 状態に変数が設定されていない場合も
 *  固定の空変数を返す.
 *
 *  @param  [in]    state   状態.
 *  @return 変数が設定されている場合は, @c state の変数のポインタが返る.
 *          設定されていない場合は, 固定の空変数のポインタが返る.
 *  @pre    state != NULL
 */
static inline struct fsm_state_variable *get_state_variable(const struct fsm_state *state)
{
    static struct fsm_state_variable null_obj = FSM_STATE_VARIABLE_INITIALIZER;
    return (state->variable != NULL) ? state->variable : &null_obj;
}

/**
 *  entry アクションが設定されていれば, 実行する.
 *
 *  entry アクションが設定されていない場合は何もしない.
 *  状態が入れ子になっている場合, 目標となる子にたどり着く途中では
 *  @c cmpl が false となる.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    state   遷移先の状態.
 *  @param  [in]    cmpl    遷移完了.
 *  @pre    machine != NULL
 *  @pre    state != NULL
 */
static inline void entry_if_can_be(struct fsm *machine,
                                   const struct fsm_state *state,
                                   bool cmpl)
{
    if (state->entry != NULL) {
        state->entry(machine, get_state_variable(state)->data, cmpl);
    }
}

/**
 *  do アクティビティが設定されていれば, 実行する.
 *
 *  do アクティビティが設定されていない場合は何もしない.
 *
 *  @param  [in]    machine 状態マシン.
 *  @pre    machine != NULL
 */
static inline void exec_if_can_be(struct fsm *machine)
{
    const struct fsm_state *state = machine->current;
    if ((state->exec != NULL)) {
        state->exec(machine, get_state_variable(state)->data);
    }
}

/**
 *  exit アクションが設定されていれば, 実行する.
 *
 *  exit アクションが設定されていない場合は何もしない.
 *  親状態の履歴状態の更新も行う.
 *  状態が入れ子になっている場合, 目標となる子にたどり着く途中では
 *  @c cmpl が false となる.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    state   遷移元の状態.
 *  @param  [in]    cmpl    遷移完了.
 *  @pre    machine != NULL
 *  @pre    state != NULL
 */
static inline void exit_if_can_be(struct fsm *machine,
                                  const struct fsm_state *state,
                                  bool cmpl)
{
    if (state->exit != NULL) {
        state->exit(machine, get_state_variable(state)->data, cmpl);
    }
    if (state->parent != NULL) {
        get_state_variable(state->parent)->history = state;
    }
}

/**
 *  状態マシンの状態を変更する.
 *
 *  @param  [in,out]    machine     状態マシン.
 *  @param  [in]        new_state   新しい状態.
 *  @pre    machine != NULL
 *  @pre    new_state != NULL
 */
static void fsm_change_state(struct fsm *machine,
                             const struct fsm_state *new_state)
{
    STACK src_ancs = machine->src_ancestors;
    STACK dest_ancs = machine->dest_ancestors;
    const struct fsm_state *src_state;
    const struct fsm_state *dest_state;
    const struct fsm_state *ancestor;
    int count;

    /* 自己遷移の場合 */
    if (machine->current == new_state) {
        exit_if_can_be(machine, machine->current, true);
        entry_if_can_be(machine, new_state, true);
        return;
    }

    /* 共通の祖先より後の出状/入状イベントを処理する.
     * e.g. 以下の場合は, f,e,d の出状イベントと g,h,i の入場イベントを処理する.
     *      a--b--c--d--e--f(現在状態)
     *             \-g--h--i(次の状態)
     */
    stack_clear(src_ancs);
    for (src_state = machine->current;
         src_state != NULL;
         src_state = src_state->parent) {

        stack_push(src_ancs, (void *)&src_state);
    }
    stack_clear(dest_ancs);
    for (dest_state = new_state;
         dest_state != NULL;
         dest_state = dest_state->parent) {

        stack_push(dest_ancs, (void *)&dest_state);
    }
    do {
        src_state = dest_state = NULL;
        stack_pop(src_ancs, &src_state);
        count = stack_pop(dest_ancs, &dest_state);
    } while (src_state == dest_state);
    assert(dest_state != NULL);
    ancestor = (src_state != NULL) ? src_state->parent : machine->current;

    for (src_state = machine->current;
         src_state != ancestor;
         src_state = src_state->parent) {

        assert(src_state != NULL);
        exit_if_can_be(machine, src_state, (src_state->parent == ancestor));
    }
DEBUG("state: %s -> %s", machine->current->name, new_state->name);
    machine->current = new_state;
    do {
        entry_if_can_be(machine, dest_state, (count == 0));
        count = stack_pop(dest_ancs, &dest_state);
    } while (count >= 0);

    /* 履歴状態に対する遷移を行う. */
    if (get_state_variable(dest_state)->history != NULL) {
        fsm_change_state(machine, get_state_variable(dest_state)->history);
    }
}

/**
 *  状態の遷移を行う.
 *
 *  遷移にガード条件が設定されている場合は, 条件を満たさない場合は
 *  遷移は行わない.
 *  遷移にアクションが設定されている場合は, アクションを実行後に遷移を行う.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    state   起点となる状態.
 *  @param  [in]    event   発生したイベント.
 *  @return 対応表に条件が一致する項目があった場合は, 状態遷移が行われ, true が返る.
 *          一致する項目がなかった場合は, false が返る.
 */
static bool fsm_state_transit(struct fsm *machine, const struct fsm_state *state, int event)
{
    int i;

    for (i = 0; machine->corresps[i].from != NULL; ++i) {
        const struct fsm_trans *corr = &machine->corresps[i];
        if ((corr->from == state) && (corr->event == event)) {
            if ((corr->cond == NULL) || corr->cond(machine)) {
                if (corr->action != NULL) {
                    corr->action(machine);
                }
                fsm_change_state(machine, corr->to);
            }
            return true;
        }
    }

    return false;
}

/**
 *  @details    開始状態の状態マシンを, 生成する.
 *
 *  @param      [in]    corresps    状態遷移の対応表.
 *  @return     成功時は, 確保および初期化されたオブジェクトのポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
struct fsm *fsm_init(const struct fsm_trans *corresps)
{
    struct fsm *machine;
    STACK src_ancs, dest_ancs;

    if (corresps == NULL) {
        errno = EINVAL;
        return NULL;
    }

    machine = malloc(sizeof(struct fsm));
    src_ancs = stack_init(sizeof(struct fsm_state*), NEST_MAX);
    dest_ancs = stack_init(sizeof(struct fsm_state*), NEST_MAX);
    if ((machine == NULL) || (src_ancs == NULL) || (dest_ancs == NULL)) {
        stack_release(dest_ancs);
        stack_release(src_ancs);
        free(machine);
        return NULL;
    }

    *machine = FSM_HELPER(&state_start, corresps, src_ancs, dest_ancs);

    /* Null 遷移を行う. */
    fsm_state_transit(machine, machine->current, EVENT_NULL_TRANSITION);

    return machine;
}

/**
 *  @details    @c machine の使用領域を解放する.
 *
 *  @param      [in,out]    machine 状態マシン.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 */
int fsm_term(struct fsm *machine)
{
    if (machine == NULL) {
        errno = EINVAL;
        return -1;
    }

    fsm_change_state(machine, &state_end);
    stack_release(machine->dest_ancestors);
    stack_release(machine->src_ancestors);
    free(machine);

    return 0;
}

/**
 *  @details    指定イベントによる状態遷移を発生させる.
 *              現在の状態に対応する遷移がない場合は, 親にイベントを伝播させる.
 *
 *  @param      [in]    machine 状態マシン.
 *  @param      [in]    event   発生したイベント.
 */
void fsm_transition(struct fsm *machine, int event)
{
    const struct fsm_state *state;

    if (machine == NULL) {
        return;
    }

    state = machine->current;
    while ((state != NULL) && !fsm_state_transit(machine, state, event)) {
        state = state->parent;
    }

    /* Null 遷移を行う. */
    fsm_state_transit(machine, machine->current, EVENT_NULL_TRANSITION);
}

/**
 *  @details    現在の状態の do アクティビティを実行する.
 *
 *  @param  [in]    machine 状態マシン.
 */
void fsm_update(struct fsm *machine)
{
    if (machine == NULL) {
        return;
    }

    exec_if_can_be(machine);
}

/**
 *  @details    指定状態の固有情報を取得する.
 *
 *  @param      [in]    state   固有情報を取得したい状態.
 *  @param      正常時は, 固有情報のポインタが返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
void *fsm_get_state_data(const struct fsm_state *state)
{
    if (state == NULL) {
        errno = EINVAL;
        return NULL;
    }

    return get_state_variable(state)->data;
}

/**
 *  @details    現在の状態の状態名を指定されたバッファにコピーする.
 *              コピーする文字列は NULL 終端を保証し, 
 *              実際にコピー可能な文字数は @c len - 1 文字となる.
 *
 *  @param      [in]    machine 状態マシン.
 *  @param      [out]   name    取得するバッファ.
 *  @param      [in]    len     取得する文字数.
 */
void fsm_current_state(struct fsm *machine, char *name, size_t len)
{
    strncpy(name, machine->current->name, len);
    name[len - 1] = '\0';
}
