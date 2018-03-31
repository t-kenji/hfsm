Sample Hierarchical Finite State Machine (HFSM) implementation {#mainpage}
==============================================================

Introduction
------------

We verify the hierarchical finite state machine which is a general implementation of state transition through sample implementation.

how to build
------------

```
$ make
```

how to test
-----------

```
$ make test
```

generate doxygen document
-------------------------

```
$ make doc
```

run clang static analyer
------------------------

```
$ scan-build --use-cc=`which clang` make NODEBUG=1 EXTRA_CFLAGS=-fblocks EXTRA_LIBS=-lBlocksRuntime
```
