/*  @file   air_conditioner.c
 *  @brief  エアコンをテーマにした実装例.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include "hfsm.h"

#define DEBUG(fmt, ...) do{fprintf(stderr, "%s:%d:%s " fmt "\n",__FILE__,__LINE__,__func__,##__VA_ARGS__);}while(0)

/*
 *  設定情報構造体.
 */
struct setting {
    int temperature;    /* 設定温度. */
};

/*
 *  冷房設定情報.
 */
static struct setting cooling_setting = {
    .temperature = 23
};

/*
 *  暖房設定情報.
 */
static struct setting heating_setting = {
    .temperature = 18
};

/*
 *  停止状態の entry アクション.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    data    状態固有情報.
 *  @param  [in]    cmpl    遷移完了.
 */
static void entry_stopped(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("エンジンを停止する");
}

/*
 *  停止状態.
 */
FSM_STATE(state_stopped, NULL, entry_stopped, NULL, NULL);

/*
 *  運転状態の entry アクション.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    data    状態固有情報.
 *  @param  [in]    cmpl    遷移完了.
 */
static void entry_running(struct fsm *machine, void *data, bool cmpl)
{
    DEBUG("エンジンを始動する.");
}

/*
 *  運転状態.
 */
FSM_STATE(state_running, NULL, entry_running, NULL, NULL);

/*
 *  冷房状態の entry アクション.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    data    状態固有情報.
 *  @param  [in]    cmpl    遷移完了.
 */
static void entry_cooling(struct fsm *machine, void *data, bool cmpl)
{
    struct setting *setting = (struct setting *)data;

    DEBUG("%d 度の冷房運転に切り替える.", setting->temperature);
}

/*
 *  冷房状態.
 */
FSM_STATE(state_cooling, &cooling_setting, entry_cooling, NULL, NULL);

/*
 *  暖房状態の entry アクション.
 *
 *  @param  [in]    machine 状態マシン.
 *  @param  [in]    data    状態固有情報.
 *  @param  [in]    cmpl    遷移完了.
 */
static void entry_heating(struct fsm *machine, void *data, bool cmpl)
{
    struct setting *setting = (struct setting *)data;

    DEBUG("%d 度の暖房運転に切り替える.", setting->temperature);
}

/*
 *  暖房状態.
 */
FSM_STATE(state_heating, &heating_setting, entry_heating, NULL, NULL);

/*
 *  運転イベント.
 */
FSM_EVENT(event_run);

/*
 *  停止イベント.
 */
FSM_EVENT(event_stop);

/*
 *  冷房切り替えイベント.
 */
FSM_EVENT(event_cooling);

/*
 *  暖房切り替えイベント.
 */
FSM_EVENT(event_heating);

/*
 *  温度上昇イベント.
 */
FSM_EVENT(event_inc_temp);

/*
 *  温度下降イベント.
 */
FSM_EVENT(event_dec_temp);

/*
 *  冷房温度上昇アクション.
 *
 *  @param  [in]    machine 状態マシン.
 */
FSM_ACTION(action_cooling_inc_temp, (struct fsm *machine))
{
    struct setting *setting = fsm_get_state_data(state_cooling);

    ++(setting->temperature);
    DEBUG("冷房の温度を %d 度に変更する.", setting->temperature);
}

/*
 *  冷房温度下降アクション.
 *
 *  @param  [in]    machine 状態マシン.
 */
FSM_ACTION(action_cooling_dec_temp, (struct fsm *machine))
{
    struct setting *setting = fsm_get_state_data(state_cooling);

    --(setting->temperature);
    DEBUG("冷房の温度を %d 度に変更する.", setting->temperature);
}

/*
 *  暖房温度上昇アクション.
 *
 *  @param  [in]    machine 状態マシン.
 */
FSM_ACTION(action_heating_inc_temp, (struct fsm *machine))
{
    struct setting *setting = fsm_get_state_data(state_heating);

    ++(setting->temperature);
    DEBUG("暖房の温度を %d 度に変更する.", setting->temperature);
}

/*
 *  暖房温度下降アクション.
 *
 *  @param  [in]    machine 状態マシン.
 */
FSM_ACTION(action_heating_dec_temp, (struct fsm *machine))
{
    struct setting *setting = fsm_get_state_data(state_heating);

    --(setting->temperature);
    DEBUG("暖房の温度を %d 度に変更する.", setting->temperature);
}

/*
 *  実行ログは以下のようになる.
 *  @code
 *  $ ./example/air_conditioner 
 *  hfsm.c:245(fsm_state_transit) state: start --null-> state_stopped
 *  air_conditioner.c:44:entry_stopped エンジンを停止する
 *  hfsm.c:245(fsm_state_transit) state: state_stopped --event_run-> state_running
 *  air_conditioner.c:61:entry_running エンジンを始動する.
 *  air_conditioner.c:80:entry_cooling 23 度の冷房運転に切り替える.
 *  air_conditioner.c:147:action_cooling_inc_temp_func 冷房の温度を 24 度に変更する.
 *  hfsm.c:259(fsm_state_transit) state: state_cooling event_inc_temp/action_cooling_inc_temp
 *  hfsm.c:245(fsm_state_transit) state: state_cooling --event_heating-> state_heating
 *  air_conditioner.c:99:entry_heating 18 度の暖房運転に切り替える.
 *  air_conditioner.c:186:action_heating_dec_temp_func 暖房の温度を 17 度に変更する.
 *  hfsm.c:259(fsm_state_transit) state: state_heating event_dec_temp/action_heating_dec_temp
 *  hfsm.c:245(fsm_state_transit) state: state_running --event_stop-> state_stopped
 *  air_conditioner.c:44:entry_stopped エンジンを停止する
 *  hfsm.c:245(fsm_state_transit) state: state_stopped --event_run-> state_running
 *  air_conditioner.c:61:entry_running エンジンを始動する.
 *  air_conditioner.c:99:entry_heating 17 度の暖房運転に切り替える.
 *  @endcode
 */
int main(int argc, char **argv)
{
    const struct fsm_rels rels[] = {
        /* 運転状態のデフォルトは冷房とする. */
        FSM_RELS_HELPER(state_cooling, state_running, true),
        FSM_RELS_HELPER(state_heating, state_running, false),
        FSM_RELS_TERMINATOR
    };
    const struct fsm_trans corresps[] = {
        /* 初期状態は "停止" 状態とする. */
        FSM_TRANS_HELPER(state_start, event_null, NULL, NULL, state_stopped),
        /* "停止" 状態で "運転" イベントを受けると "運転" 状態にする. */
        FSM_TRANS_HELPER(state_stopped, event_run, NULL, NULL, state_running),
        /* "運転" (及びその子である "冷房" or "暖房" ) 状態で "停止" イベントを受けると "停止" 状態にする. */
        FSM_TRANS_HELPER(state_running, event_stop, NULL, NULL, state_stopped),
        /* "停止" 状態で "冷房" イベントを受けると "冷房" 状態にする. */
        FSM_TRANS_HELPER(state_stopped, event_cooling, NULL, NULL, state_cooling),
        /* "停止" 状態で "暖房" イベントを受けると "暖房" 状態にする. */
        FSM_TRANS_HELPER(state_stopped, event_heating, NULL, NULL, state_heating),
        /* "冷房" 状態で "暖房" イベントを受けると "暖房" 状態にする. */
        FSM_TRANS_HELPER(state_cooling, event_heating, NULL, NULL, state_heating),
        /* "暖房" 状態で "冷房" イベントを受けると "冷房" 状態にする. */
        FSM_TRANS_HELPER(state_heating, event_cooling, NULL, NULL, state_cooling),
        /* "冷房" 状態で "温度上昇" イベントを受けると "冷房温度上昇" アクションを実行する. */
        FSM_TRANS_HELPER(state_cooling, event_inc_temp, NULL, action_cooling_inc_temp, NULL),
        /* "冷房" 状態で "温度下降" イベントを受けると "冷房温度下降" アクションを実行する. */
        FSM_TRANS_HELPER(state_cooling, event_dec_temp, NULL, action_cooling_dec_temp, NULL),
        /* "暖房" 状態で "温度上昇" イベントを受けると "暖房温度上昇" アクションを実行する. */
        FSM_TRANS_HELPER(state_heating, event_inc_temp, NULL, action_heating_inc_temp, NULL),
        /* "暖房" 状態で "温度下降" イベントを受けると "暖房温度下降" アクションを実行する. */
        FSM_TRANS_HELPER(state_heating, event_dec_temp, NULL, action_heating_dec_temp, NULL),
        FSM_TRANS_TERMINATOR
    };

    struct fsm *machine = fsm_init(rels, corresps);

    /* 運転を開始する. */
    fsm_transition(machine, event_run);

    /* 温度を上げる. */
    fsm_transition(machine, event_inc_temp);

    /* 暖房に切り替える. */
    fsm_transition(machine, event_heating);

    /* 温度を下げる. */
    fsm_transition(machine, event_dec_temp);

    /* 運転を停止する. */
    fsm_transition(machine, event_stop);

    /* 運転を再開する. */
    fsm_transition(machine, event_run);

    fsm_term(machine);

    return 0;
}
