# Sample Hierarchical Finite State Machine (HFSM) implementation

## Introduction

We verify the hierarchical finite state machine which is a general implementation of state transition through sample implementation.

状態遷移の汎用実装である階層型有限状態機械をサンプル実装を通して検証します.

## run clang static analyer

```
$ scan-build --use-cc=`which clang` make NODEBUG=1 EXTRA_CFLAGS=-fblocks EXTRA_LIBS=-lBlocksRuntime
```
