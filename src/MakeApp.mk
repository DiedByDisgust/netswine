PROGRAM_NAME := NetSwine

PREFIX := /usr
bin := $(PREFIX)/bin

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14

OBJS = ui.o line.o reader.o

NCURSES_LIBS?=-lncurses

all: netswine service

.PHONY: install uninstall clean

uninstall:
	rm $(DESTDIR)$(bin)/netswine || true
	rm $(DESTDIR)$(bin)/netswined || true
	rm $(DESTDIR)/etc/systemd/system/netswined.service || true

install:
	install -d -m 755 $(DESTDIR)$(bin)
	install -d -m 755 $(DESTDIR)/etc/systemd/system
	install -m 755 netswine $(DESTDIR)$(bin)
	install -m 755 netswined $(DESTDIR)$(bin)
	install -m 644 netswined.service $(DESTDIR)/etc/systemd/system
	@echo
	@echo "Installed NetSwine daemon and reader to $(DESTDIR)$(bin)"
	@echo

netswine: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) main.cpp $(OBJS) -o netswine -lsqlite3 ${NCURSES_LIBS} -DVERSION=\"$(VERSION)\"
service: service.cpp
	$(CXX) $(CXXFLAGS) service.cpp -lsqlite3 -l:libnethogs.so -o netswined -DVERSION=\"$(VERSION)\" -DPROGRAM_NAME=\"$(PROGRAM_NAME)\"

ui.o: ui.cpp ui.h
	$(CXX) $(CXXFLAGS) -c ui.cpp -DVERSION=\"$(VERSION)\" -DPROGRAM_NAME=\"$(PROGRAM_NAME)\"
line.o: line.cpp line.h
	$(CXX) $(CXXFLAGS) -c line.cpp
reader.o: reader.cpp reader.h
	$(CXX) $(CXXFLAGS) -c reader.cpp

clean:
	rm -f $(OBJS)
	rm -f netswine
	rm -f netswined

