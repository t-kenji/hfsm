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

SCENARIO("ツリーが初期化できること", "[tree][init]") {
    GIVEN("特になし") {
        WHEN("ツリーを容量 5 で初期化する") {
            TREE tree = tree_init(sizeof(int), 5);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(tree != NULL);
            }

            tree_release(tree);
        }
    }
}

SCENARIO("ツリーに要素が追加できること", "[tree][add]") {
    GIVEN("ツリーを容量 5 で初期化しておく") {
        TREE tree = tree_init(sizeof(int), 5);

        WHEN("要素を追加しない") {
            THEN("ツリーの要素数が 0 であること") {
                REQUIRE(tree_count(tree) == 0);
            }
        }

        WHEN("要素を root に 1 つ追加する") {
            int a = 0x55;
            REQUIRE(tree_insert(tree, NULL, &a) != NULL);

            THEN("ツリーの要素数が 1 であること") {
                REQUIRE(tree_count(tree) == 1);
            }
        }

        WHEN("要素を root に 5 つ追加する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(tree_insert(tree, NULL, &i) != NULL);
            }

            THEN("ツリーの要素数が 5 であること") {
                REQUIRE(tree_count(tree) == 5);
            }
        }

        WHEN("要素を階層的に 5 つ追加する") {
            int a = 0, b;
            REQUIRE(tree_insert(tree, NULL, &a) != NULL);
            a = 1; b = 0;
            REQUIRE(tree_insert(tree, &b, &a) != NULL);
            a = 2; b = 0;
            REQUIRE(tree_insert(tree, &b, &a) != NULL);
            a = 3; b = 2;
            REQUIRE(tree_insert(tree, &b, &a) != NULL);
            a = 4; b = 3;
            REQUIRE(tree_insert(tree, &b, &a) != NULL);

            THEN("ツリーの要素数が 5 であること") {
                REQUIRE(tree_count(tree) == 5);
            }
        }

        tree_release(tree);
    }
}

SCENARIO("ツリーを反復子で処理できること", "[tree][iterator]") {
    GIVEN("ツリーを容量 5 で初期化しておく") {
        TREE tree = tree_init(sizeof(int), 5);

        WHEN("要素を root に 5 つ追加する") {
            for (int i = 0; i < 5; ++i) {
                tree_insert(tree, NULL, &i);
            }

            THEN("反復子でツリーに沿った順に値が取得できること") {
                TREE_ITER iter = tree_iter_get(tree);
                REQUIRE(iter != NULL);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 0);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 1);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 2);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 3);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 4);
            }
        }

        WHEN("要素を階層的に 5 つ追加する (pat 1)") {
            int a = 0, b;
            tree_insert(tree, NULL, &a);
            a = 1; b = 0;
            tree_insert(tree, &b, &a);
            a = 2; b = 0;
            tree_insert(tree, &b, &a);
            a = 3; b = 2;
            tree_insert(tree, &b, &a);
            a = 4; b = 3;
            tree_insert(tree, &b, &a);

            THEN("反復子でツリーに沿った順に値が取得できること") {
                TREE_ITER iter = tree_iter_get(tree);
                REQUIRE(iter != NULL);

                REQUIRE(*(int *)tree_iter_get_payload(iter) == 0);
                REQUIRE(tree_iter_get_age(iter) == 1);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 1);
                REQUIRE(tree_iter_get_age(iter) == 2);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 2);
                REQUIRE(tree_iter_get_age(iter) == 2);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 3);
                REQUIRE(tree_iter_get_age(iter) == 3);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 4);
                REQUIRE(tree_iter_get_age(iter) == 4);

                tree_iter_release(iter);
            }
        }

        WHEN("要素を階層的に 5 つ追加する (pat 2)") {
            int a = 0, b;
            tree_insert(tree, NULL, &a);
            a = 1; b = 0;
            tree_insert(tree, &b, &a);
            a = 2; b = 0;
            tree_insert(tree, &b, &a);
            a = 3; b = 2;
            tree_insert(tree, &b, &a);
            a = 4; b = 1;
            tree_insert(tree, &b, &a);

            THEN("反復子でツリーに沿った順に値が取得できること") {
                TREE_ITER iter = tree_iter_get(tree);
                REQUIRE(iter != NULL);

                REQUIRE(*(int *)tree_iter_get_payload(iter) == 0);
                REQUIRE(tree_iter_get_age(iter) == 1);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 1);
                REQUIRE(tree_iter_get_age(iter) == 2);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 4);
                REQUIRE(tree_iter_get_age(iter) == 3);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 2);
                REQUIRE(tree_iter_get_age(iter) == 2);
                iter = tree_iter_next(iter);
                REQUIRE(*(int *)tree_iter_get_payload(iter) == 3);
                REQUIRE(tree_iter_get_age(iter) == 3);

                tree_iter_release(iter);
            }
        }

        tree_release(tree);
    }
}
