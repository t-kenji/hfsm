# makefile for hfsm sample implementation test.

include ../config.mk

TARGET = $(NAME)_test

INCS = -I. -I../include -I../src
ifneq ($(CATCH2_DIR),)
  INCS += -I$(CATCH2_DIR)
endif
OPT_WARN = -Wall -Werror
OPT_OPTIM = -Og
OPT_DBG = -g
OPT_DEP = -MMD -MP
EXTRA_DEFS =
EXTRA_LIBS =

OPTS = $(OPT_WARN) $(OPT_OPTIM) $(OPT_DBG) $(OPT_DEP)
CFLAGS = -std=c++11 $(OPTS) $(INCS)
CPPFLAGS = $(EXTRA_DEFS)
LDFLAGS =
LIBS = ../src/lib$(NAME).a $(EXTRA_LIBS)

SRCS = main.cpp collections.cpp hfsm.cpp
DEPS = $(SRCS:.cpp=.d)
OBJS = $(SRCS:.cpp=.o)

.PHONY: all $(TARGET) clean

%.o: %.cpp
	$(QCXX)$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(QLINK)$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(QCLEAN)rm -rf $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
