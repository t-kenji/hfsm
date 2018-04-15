# makefile for hfsm sample implementation library.

include ../config.mk

TARGET = lib$(NAME).a

INCS = -I. -I../include
OPT_WARN = -Wall -Werror
OPT_OPTIM = -Og
OPT_DBG = -g
OPT_DEP = -MMD -MP
EXTRA_DEFS =
EXTRA_CFLAGS =
EXTRA_LIBS =

OPTS = $(OPT_WARN) $(OPT_OPTIM) $(OPT_DBG) $(OPT_DEP)
CFLAGS = -std=c11 $(OPTS) $(INCS) $(EXTRA_CFLAGS)
CPPFLAGS = -DNODEBUG=$(NODEBUG) $(EXTRA_DEFS)
LDFLAGS = -X -r
LIBS = $(EXTRA_LIBS)

SRCS = collections.c hfsm.c
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o)

.PHONY: all $(TARGET) clean

%.o: %.c
	$(QCC)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(QLD)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(QCLEAN)rm -rf $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
