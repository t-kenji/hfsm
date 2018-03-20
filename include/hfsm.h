/** @file   hfsm.h
 *  @brief  Hierarchical Finite State Machine (HFSM) implementation.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-19 create new.
 */
#ifndef __HFSM_H__
#define __HFSM_H__

#include <stdbool.h>

#include "collections.h"

struct fsm_trans;
struct fsm;

/**
 *  Initialize FSM instance.
 */
struct fsm *fsm_init(const struct fsm_trans *corresps);

/**
 *  Terminate FSM instance.
 */
int fsm_term(struct fsm *machine);

/**
 *  Transition by event.
 */
void fsm_transition(struct fsm *machine, int event);

/**
 *  State do execution.
 */
void fsm_update(struct fsm *machine);

/**
 *  Variable structure attached to state.
 */
struct fsm_state_variable {
    const struct fsm_state *history; /**< State's history */
    void *data;                      /**< State specific variable */
};

/**
 *  Variable structure initializer.
 */
#define FSM_STATE_VARIABLE_INITIALIZER (struct fsm_state_variable){ \
    .history = NULL,                                                \
    .data = NULL                                                    \
}

/**
 *  State structure.
 */
struct fsm_state {
    const char *name;                                                 /**< State name used for debugging */
    const struct fsm_state * const parent;                            /**< Parent state */
    struct fsm_state_variable *variable;                              /**< State specific variable */

    void (* const entry)(struct fsm *machine, void *data, bool cmpl); /**< Function for state entry */
    void (* const exec)(struct fsm *machine, void *data);             /**< Function for state do */
    void (* const exit)(struct fsm *machine, void *data, bool cmpl);  /**< Function for state exit */
};

/**
 *  State initialization helper.
 */
#define FSM_STATE_HELPER(nam, prnt, var, ent, exe, exi) { \
    .name = (nam),                                        \
    .parent = (prnt),                                     \
    .variable = (var),                                    \
    .entry = (ent),                                       \
    .exec = (exe),                                        \
    .exit = (exi)                                         \
}

/**
 *  State structure initializer.
 */
#define FSM_STATE_INITIALIZER(nam) \
    (struct fsm_state)FSM_STATE_HELPER((nam), NULL, NULL, NULL, NULL, NULL)

/**
 *  State definition helper.
 */
#define FSM_STATE(var, prnt, dat, ent, exe, exi)   \
    static struct fsm_state_variable var##_var = { \
        .history = NULL,                           \
        .data = (dat)                              \
    };                                             \
    static const struct fsm_state var =            \
        FSM_STATE_HELPER(#var, (prnt), &var##_var, (ent), (exe), (exi))

/**
 *  Start state. (It is the initial state of the state machine)
 */
extern const struct fsm_state state_start;

/**
 *  Transition structure.
 */
struct fsm_trans {
    const struct fsm_state *from; /**< Current state */
    int event;                    /**< Trigger event */
    void (*action)(struct fsm *); /**< Function for transition action */
    const struct fsm_state *to;   /**< State after transition */
};

/**
 *  Transition initialization helper.
 */
#define FSM_TRANS_HELPER(f, e, a, t) { \
    .from = (f),                       \
    .event = (e),                      \
    .action = (a),                     \
    .to = (t)                          \
}

/**
 *  Transition structure initializer.
 */
#define FSM_TRANS_INITIALIZER \
    (struct fsm_trans)FSM_TRANS_HELPER(NULL, -1, NULL, NULL)

/**
 *  Transition corresponding array terminator.
 */
#define FSM_TRANS_TERMINATOR FSM_TRANS_INITIALIZER

#endif /* __HFSM_H__ */
