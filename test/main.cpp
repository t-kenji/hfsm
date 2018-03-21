/** @file   main.cpp
 *  @brief  Catch2 スタートアップ処理.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-22 新規作成.
 */
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

/**
 *  スタートアップ.
 *
 *  @param  [in]    argc    引数の数.
 *  @param  [in]    argv    引数の文字列配列.
 *  @return 成功時には 0 が返り, 失敗時には 1 が返る.
 */
int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}
