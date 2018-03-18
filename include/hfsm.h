#ifndef __HFSM_H__
#define __HFSM_H__

#include <stdbool.h>

struct fsm_state;
struct fsm_trans;

struct fsm {
    const struct fsm_state *current;
    const struct fsm_trans *corresps;
};

extern const struct fsm_state state_start;

int fsm_init(struct fsm *machine, const struct fsm_trans *corresps);
int fsm_term(struct fsm *machine);
void fsm_transition(struct fsm *machine, int event);
void fsm_update(struct fsm *machine);

struct fsm_state_variable {
    const struct fsm_state *history;
    void *data;
};

#define FSM_STATE_VARIABLE_INITIALIZER (struct fsm_state_variable){ \
    .history = NULL,                                                \
    .data = NULL                                                    \
}

struct fsm_state {
    const char *name;
    const struct fsm_state * const parent;
    struct fsm_state_variable *variable;

    void (* const entry)(struct fsm *machine, void *data, bool cmpl);
    void (* const exec)(struct fsm *machine, void *data);
    void (* const exit)(struct fsm *machine, void *data, bool cmpl);
};

#define FSM_STATE_HELPER(nam, prnt, var, ent, exe, exi) { \
    .name = (nam),                                        \
    .parent = (prnt),                                     \
    .variable = (var),                                    \
    .entry = (ent),                                       \
    .exec = (exe),                                        \
    .exit = (exi)                                         \
}

#define FSM_STATE_INITIALIZER(nam) \
    (struct fsm_state)FSM_STATE_HELPER((nam), NULL, NULL, NULL, NULL, NULL)

#define FSM_STATE(var, prnt, dat, ent, exe, exi)   \
    static struct fsm_state_variable var##_var = { \
        .history = NULL,                           \
        .data = (dat)                              \
    };                                             \
    static const struct fsm_state var =            \
        FSM_STATE_HELPER(#var, (prnt), &var##_var, (ent), (exe), (exi))

struct fsm_trans {
    const struct fsm_state *from;
    int event;
    void (*action)(struct fsm *);
    const struct fsm_state *to;
};

#define FSM_TRANS_HELPER(f, e, a, t) { \
    .from = (f),                       \
    .event = (e),                      \
    .action = (a),                     \
    .to = (t)                          \
}

#define FSM_TRANS_INITIALIZER \
    (struct fsm_trans)FSM_TRANS_HELPER(NULL, -1, NULL, NULL)

#define FSM_TRANS_TERMINATOR FSM_TRANS_INITIALIZER

#endif /* __HFSM_H__ */
