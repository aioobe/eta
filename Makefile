CC = gcc
CFLAGS ?= -Wall
SRC := src
OBJ := obj

PREFIX ?= /usr/local

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

eta: $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(SRC) -c $< $(LDFLAGS) -o $@

srcdist:
	tar cfz eta-1.0.tar.gz --transform 's,^,eta-1.0/,' docs/ LICENSE Makefile src/

install: eta
	install -D eta \
	        $(DESTDIR)$(PREFIX)/bin/eta
	install -m 664 -D docs/eta.1 \
	        $(DESTDIR)$(PREFIX)/share/man/man1/eta.1

uninstall:
	-rm -f $(DESTDIR)$(PREFIX)/bin/eta \
	       $(DESTDIR)$(PREFIX)/share/man/man1/eta.1

distclean: clean

clean:
	-@rm -rf $(OBJ) ./eta

.PHONY: clean srcdist install uninstall
