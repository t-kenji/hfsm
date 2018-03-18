/** @file   hfsm.c
 *  @brief  Hierarchical Finite State Machine (HFSM) implementation.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#include "debug.h"
#include "collections.h"
#include "hfsm.h"

const struct fsm_state state_start = FSM_STATE_INITIALIZER("start");

static const struct fsm_state state_end = FSM_STATE_INITIALIZER("end");

static inline struct fsm_state_variable *get_state_variable(const struct fsm_state *state)
{
    static struct fsm_state_variable null_obj = FSM_STATE_VARIABLE_INITIALIZER;
    return (state->variable != NULL) ? state->variable : &null_obj;
}

static inline void entry_if_can_be(struct fsm *machine,
                                   const struct fsm_state *state,
                                   bool cmpl)
{
    if ((state != NULL) && (state->entry != NULL)) {
        state->entry(machine, get_state_variable(state)->data, cmpl);
    }
}

static inline void exec_if_can_be(struct fsm *machine,
                                  const struct fsm_state *state)
{
    if ((state != NULL) && (state->exec != NULL)) {
        state->exec(machine, get_state_variable(state)->data);
    }
}

static inline void exit_if_can_be(struct fsm *machine,
                                  const struct fsm_state *state,
                                  bool cmpl)
{
    if ((state != NULL) && (state->exit != NULL)) {
        state->exit(machine, get_state_variable(state)->data, cmpl);
        if (state->parent != NULL) {
            get_state_variable(state->parent)->history = state;
        }
    }
}

static void fsm_change_state(struct fsm *machine, const struct fsm_state *new_state)
{
    LIST src_ancestors, dest_ancestors;
    const struct fsm_state *src_state, *dest_state;
    LIST_ITER iter;
    const struct fsm_state *ancestor;

    /* 自己遷移 */
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
    list_init(&src_ancestors, sizeof(struct fsm_state*));
    for (src_state = machine->current;
         src_state != NULL;
         src_state = src_state->parent) {

        list_push(&src_ancestors, (void *)&src_state);
    }
    list_init(&dest_ancestors, sizeof(struct fsm_state*));
    for (dest_state = new_state;
         dest_state != NULL;
         dest_state = dest_state->parent) {

        list_push(&dest_ancestors, (void *)&dest_state);
    }
    do {
        list_pop(&src_ancestors, &src_state);
        list_pop(&dest_ancestors, &dest_state);
    } while (src_state == dest_state);
    ancestor = (src_state != NULL) ? src_state->parent : machine->current;
    list_release(&src_ancestors);

    for (src_state = machine->current;
         src_state != ancestor;
         src_state = src_state->parent) {

        exit_if_can_be(machine, src_state, (src_state->parent == ancestor));
    }
DEBUG("state: %s -> %s", machine->current->name, new_state->name);
    machine->current = new_state;
    entry_if_can_be(machine, dest_state, (dest_ancestors.length == 0));
    for (iter = list_iter(&dest_ancestors);
         iter != NULL;
         iter = list_next(iter)) {

        dest_state = *(const struct fsm_state **)list_get_payload(iter);
        entry_if_can_be(machine, dest_state, (list_next(iter) == NULL));
    }

    list_release(&dest_ancestors);

    /* 履歴状態に対する遷移を行う */
    if (get_state_variable(dest_state)->history != NULL) {
        fsm_change_state(machine, get_state_variable(dest_state)->history);
    }
}

int fsm_init(struct fsm *machine, const struct fsm_trans *corresps)
{
    machine->current = &state_start;
    machine->corresps = corresps;
    return 0;
}

int fsm_term(struct fsm *machine)
{
    fsm_change_state(machine, &state_end);
    return 0;
}

static bool fsm_state_transit(struct fsm *machine, const struct fsm_state *state, int event)
{
    int i;

    for (i = 0; machine->corresps[i].from != NULL; ++i) {
        const struct fsm_trans *corr = &machine->corresps[i];
        if ((corr->from == state) && (corr->event == event)) {
            if (corr->action != NULL) {
                corr->action(machine);
            }
            fsm_change_state(machine, corr->to);
            return true;
        }
    }

    return false;
}

void fsm_transition(struct fsm *machine, int event)
{
    const struct fsm_state *state = machine->current;

    while ((state != NULL) && !fsm_state_transit(machine, state, event)) {
        state = state->parent;
    }
}

void fsm_update(struct fsm *machine)
{
    exec_if_can_be(machine, machine->current);
}
