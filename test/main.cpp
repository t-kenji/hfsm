/** @file   main.cpp
 *  @brief  HFSM test main.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#include <cstdio>

extern "C" {
#include "debug.h"
#include "collections.h"
#include "hfsm.h"
}

struct parent_param {
    int param;
};

static struct parent_param parent_param = {100};

void parent_entry(struct fsm *machine, void *data, bool cmpl)
{
    struct parent_param *param = (struct parent_param *)data;
    DEBUG("Hello(%d) %d", cmpl, param->param);
}

void parent_exec(struct fsm *machine, void *data)
{
    DEBUG("Execute");
}

void parent_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}

FSM_STATE(state_parent, NULL, &parent_param, parent_entry, parent_exec, parent_exit);

void dummy_entry(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Hello(%d)", cmpl);
}

void dummy_exec(struct fsm *machine, void *data)
{
    DEBUG("Execute");
}

void dummy_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}

FSM_STATE(state_dummy, &state_parent, NULL, dummy_entry, dummy_exec, dummy_exit);

void state4_entry(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Hello(%d)", cmpl);
}
void state4_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}
FSM_STATE(state_state4, &state_parent, NULL, state4_entry, NULL, state4_exit);

void state0_entry(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Hello(%d)", cmpl);
}
void state0_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}
FSM_STATE(state_state0, &state_state4, NULL, state0_entry, NULL, state0_exit);

void state1_entry(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Hello(%d)", cmpl);
}
void state1_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}
FSM_STATE(state_state1, &state_state4, NULL, state1_entry, NULL, state1_exit);

void state2_entry(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Hello(%d)", cmpl);
}
void state2_exit(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("Bye Bye(%d)", cmpl);
}
FSM_STATE(state_state2, &state_state1, NULL, state2_entry, NULL, state2_exit);

static void action_st0_to_st2(struct fsm *machine)
{
    DEBUG("Action!");
}

int main(int argc, char **argv)
{
    static const struct fsm_trans corresps[] = {
        FSM_TRANS_HELPER(&state_start, 0, NULL, &state_dummy),
        FSM_TRANS_HELPER(&state_dummy, 1, NULL, &state_state0),
        FSM_TRANS_HELPER(&state_state0, 2, action_st0_to_st2, &state_state2),
        FSM_TRANS_HELPER(&state_state1, 3, NULL, &state_state0),
        FSM_TRANS_HELPER(&state_state0, 4, NULL, &state_state1),
        FSM_TRANS_TERMINATOR
    };
    static struct fsm machine;

    fsm_init(&machine, corresps);
    fsm_transition(&machine, 0);
    fsm_transition(&machine, 1);
    fsm_transition(&machine, 2);
    fsm_transition(&machine, 3);
    fsm_transition(&machine, 4);
    fsm_term(&machine);

    return 0;
}
