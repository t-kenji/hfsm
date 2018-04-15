# makefile for hfsm sample implementation example.

include ../config.mk

TARGETS = air_conditioner

INCS = -I. -I../include -I../src
OPT_WARN = -Wall -Werror
OPT_OPTIM = -Og
OPT_DBG = -g
OPT_DEP = -MMD -MP
EXTRA_DEFS =
EXTRA_LIBS =

OPTS = $(OPT_WARN) $(OPT_OPTIM) $(OPT_DBG) $(OPT_DEP)
CFLAGS = -std=c11 $(OPTS) $(INCS)
CPPFLAGS = $(EXTRA_DEFS)
LDFLAGS =
LIBS = ../src/lib$(NAME).a $(EXTRA_LIBS)

DEPS = $(TARGETS:=.d)

.PHONY: all $(TARGETS) clean

%.o: %.c
	$(QCC)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all: $(TARGETS)

air_conditioner: air_conditioner.o
	$(QLINK)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(QCLEAN)rm -rf $(DEPS) $(TARGETS)

-include $(DEPS)
