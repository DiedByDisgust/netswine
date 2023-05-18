export VERSION := $(shell ./determineVersion.sh)

all:
	$(MAKE) -C src -f MakeApp.mk $@

check:
	$(MAKE) -C src -f MakeApp.mk $@

install:
	$(MAKE) -C src -f MakeApp.mk $@

uninstall:
	$(MAKE) -C src -f MakeApp.mk $@

netswine:
	$(MAKE) -C src -f MakeApp.mk $@

service:
	$(MAKE) -C src -f MakeApp.mk $@

clean:
	$(MAKE) -C src -f MakeApp.mk $@

format:
	clang-format -i *.cpp *.h
