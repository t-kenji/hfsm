/** @file   debug.h
 *  @brief  provide functions related to debugging.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 create new.
 */
#ifndef __HFSM_DEBUG_H__
#define __HFSM_DEBUG_H__

#include <stdio.h>

/**
 *  デバッグ出力マクロ.
 *
 *  @param  [in]    format  出力文字列書式.
 *  @param  [in]    ...     書式パラメータ.
 */
#define DEBUG(format, ...)                                                                      \
    do {                                                                                        \
        fprintf(stderr, "%s:%d(%s) " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)

#endif /* __HFSM_DEBUG_H__ */
