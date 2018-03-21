/** @file   collections.cpp
 *  @brief  コレクションのテスト.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 */
#include <catch.hpp>

extern "C" {
#include "debug.h"
#include "collections.h"
}

SCENARIO("リストが初期化できること", "[list][init]") {
    GIVEN("特になし") {
        WHEN("リストを初期化する") {
            LIST list = list_init(sizeof(int), 5);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(list != NULL);
            }

            list_release(list);
        }
    }
}

SCENARIO("キューが初期化できること", "[queue][init]") {
    GIVEN("特になし") {
        WHEN("キューを容量 0 で初期化する") {
            QUEUE que = queue_init(sizeof(int), 0);

            THEN("インスタンスが NULL となる") {
                REQUIRE(que == NULL);
            }

            queue_release(que);
        }

        WHEN("キューを容量 5 で初期化する") {
            QUEUE que = queue_init(sizeof(int), 5);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(que != NULL);
            }

            queue_release(que);
        }

        WHEN("キューを容量 500 で初期化する") {
            QUEUE que = queue_init(sizeof(int), 500);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(que != NULL);
            }

            queue_release(que);
        }
    }
}

SCENARIO("キューに要素が追加できること", "[queue][enq]") {
    GIVEN("キューを容量 5 で初期化しておく") {
        QUEUE que = queue_init(sizeof(int), 5);

        WHEN("要素を追加しない") {
            THEN("キューの長さが 0 であること") {
                REQUIRE(queue_count(que) == 0);
            }
        }

        WHEN("要素を 1 つ追加する") {
            int a = 0x55;
            REQUIRE(queue_enq(que, &a) != NULL);

            THEN("キューの長さが 1 であること") {
                REQUIRE(queue_count(que) == 1);
            }
        }

        WHEN("要素を 5 つ追加する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(queue_enq(que, &i) != NULL);
            }

            THEN("キューの長さが 5 であること") {
                REQUIRE(queue_count(que) == 5);
            }
        }

        WHEN("要素を 6 つ追加する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(queue_enq(que, &i) != NULL);
            }

            THEN("6 つ目の追加に失敗すること") {
                int a = 0x55;
                REQUIRE(queue_enq(que, &a) == NULL);
            }

            THEN("キューの長さが 5 であること") {
                int a = 0x55;
                queue_enq(que, &a);
                REQUIRE(queue_count(que) == 5);
            }
        }

        queue_release(que);
    }
}

SCENARIO("キューから要素を取り出せること", "[queue][deq]") {
    GIVEN("キューを容量 5 で初期化しておく") {
        QUEUE que = queue_init(sizeof(int), 5);

        WHEN("要素を追加しない") {
            THEN("要素が取り出せないこと") {
                int a = -1;
                REQUIRE(queue_deq(que, &a) == -1);
            }
        }

        WHEN("要素を 1 つ追加する") {
            int a = 0x55;
            REQUIRE(queue_enq(que, &a) != NULL);

            THEN("追加順に要素が取り出せること") {
                int b = 0;
                REQUIRE(queue_deq(que, &b) == 0);
                REQUIRE(b == 0x55);
            }
        }

        WHEN("要素を 5 つ追加する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(queue_enq(que, &i) != NULL);
            }

            THEN("追加順に要素が取り出せること") {
                for (int i = 0; i < 5; ++i) {
                    int b = 0;
                    REQUIRE(queue_deq(que, &b) == (4 - i));
                    REQUIRE(b == i);
                }
            }
        }

        WHEN("要素の追加と取り出しを繰り返す") {
            int a = 0;
            REQUIRE(queue_enq(que, &a) != NULL);
            a++;

            THEN("追加順に要素が取り出せること") {
                int b = 0;
                REQUIRE(queue_deq(que, &b) == 0);
                REQUIRE(b == 0);

                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_deq(que, &b) == 1);
                REQUIRE(b == 1);
                REQUIRE(queue_deq(que, &b) == 0);
                REQUIRE(b == 2);

                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_deq(que, &b) == 2);
                REQUIRE(b == 3);
                REQUIRE(queue_deq(que, &b) == 1);
                REQUIRE(b == 4);
                REQUIRE(queue_deq(que, &b) == 0);
                REQUIRE(b == 5);

                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_enq(que, &a) != NULL);
                a++;
                REQUIRE(queue_deq(que, &b) == 3);
                REQUIRE(b == 6);
                REQUIRE(queue_deq(que, &b) == 2);
                REQUIRE(b == 7);
                REQUIRE(queue_deq(que, &b) == 1);
                REQUIRE(b == 8);
                REQUIRE(queue_deq(que, &b) == 0);
                REQUIRE(b == 9);
            }
        }

        queue_release(que);
    }
}
