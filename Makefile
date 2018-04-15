# makefile for hfsm sample implementasion.

include ./config.mk

.PHONY: all test example doc cppcheck oclint flawfinder clean

all:
	@make -C src

test: all
	@make -C test
	@./test/$(NAME)_test $(TAGS)

example: all
	@make -C example

doc:
	@sed -e 's/@PROJECT@/$(DOXY_PROJECT)/' \
	     -e 's/@VERSION@/$(VERSION)/' \
	     -e 's/@BRIEF@/$(DOXY_BRIEF)/' \
	     -e 's/@OUTPUT@/$(DOXY_OUTPUT)/' \
	     -e 's/@SOURCES@/$(DOXY_SOURCES)/' \
	     Doxygen.conf.in > Doxygen.conf
	@doxygen Doxygen.conf

cppcheck:
	@cppcheck --enable=all --suppress=unusedFunction -I./include ./src

oclint:
	@oclint -enable-global-analysis \
	        -enable-clang-static-analyzer \
	        -disable-rule=LongLine \
	        -disable-rule=ShortVariableName \
	        -disable-rule=UselessParentheses \
	        $(shell ls ./src/*.c) \
	        -- -I./src -I./include

flawfinder:
	@flawfinder ./include ./src

clean:
	@rm -rf Doxygen.conf $(DOXY_OUTPUT) *.plist
	@make -C src clean
	@make -C test clean
	@make -C example clean
