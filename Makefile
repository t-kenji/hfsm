# makefile for hfsm sample implementasion.

include ./config.mk

.PHONY: all test doc clean

all:
	@make -C src

test: all
	@make -C test
	@./test/$(NAME)_test $(TAGS)

doc:
	@sed -e 's/@PROJECT@/$(DOXY_PROJECT)/' \
	     -e 's/@VERSION@/$(VERSION)/' \
	     -e 's/@BRIEF@/$(DOXY_BRIEF)/' \
	     -e 's/@OUTPUT@/$(DOXY_OUTPUT)/' \
	     -e 's/@SOURCES@/$(DOXY_SOURCES)/' \
	     Doxygen.conf.in > Doxygen.conf
	@doxygen Doxygen.conf

clean:
	@rm -rf Doxygen.conf $(DOXY_OUTPUT)
	@make -C src clean
	@make -C test clean
