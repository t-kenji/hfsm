/** @file   debug.h
 *  @brief  デバッグに関する機能を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
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
#if NODEBUG == 0
#define DEBUG(format, ...)                                                                      \
    do {                                                                                        \
        fprintf(stderr, "%s:%d(%s) " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while(0)
#else
#define DEBUG(format, ...)
#endif

#endif /* __HFSM_DEBUG_H__ */
