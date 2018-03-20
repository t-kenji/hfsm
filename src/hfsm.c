/** @file   hfsm.c
 *  @brief  Hierarchical Finite State Machine (HFSM) implementation.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "hfsm.h"

struct fsm {
    const struct fsm_state *current;
    const struct fsm_trans *corresps;

    STACK src_ancestors;
    STACK dest_ancestors;
};

#define FSM_HELPER(curr, corr, s, d) (struct fsm){ \
    .current = (curr),                             \
    .corresps = (corr),                            \
    .src_ancestors = (s),                          \
    .dest_ancestors = (d)                          \
}

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
    STACK src_ancs = machine->src_ancestors;
    STACK dest_ancs = machine->dest_ancestors;
    const struct fsm_state *src_state;
    const struct fsm_state *dest_state;
    const struct fsm_state *ancestor;
    int count;

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

    /* 履歴状態に対する遷移を行う */
    if (get_state_variable(dest_state)->history != NULL) {
        fsm_change_state(machine, get_state_variable(dest_state)->history);
    }
}

struct fsm *fsm_init(const struct fsm_trans *corresps)
{
    struct fsm *machine = malloc(sizeof(struct fsm));
    STACK src_ancs = stack_init(sizeof(struct fsm_state*), 5);
    STACK dest_ancs = stack_init(sizeof(struct fsm_state*), 5);

    if ((machine == NULL) || (src_ancs == NULL) || (dest_ancs == NULL)) {
        stack_release(dest_ancs);
        stack_release(src_ancs);
        free(machine);
        return NULL;
    }

    *machine = FSM_HELPER(&state_start, corresps, src_ancs, dest_ancs);

    return machine;
}

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
