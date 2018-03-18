# makefile for hfsm sample implementasion.

include ./config.mk

.PHONY: all test clean

all:
	@make -C src

test: all
	@make -C test
	@./test/$(NAME)_test

clean:
	@make -C src clean
	@make -C test clean
