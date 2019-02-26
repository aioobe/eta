CC = gcc
SRC := src
OBJ := obj

prefix = /usr/local

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

eta: $(OBJECTS)
	$(CC) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) -I$(SRC) -c $< -o $@

srcdist:
	tar cfz eta-1.0.tar.gz --transform 's,^,eta-1.0/,' docs/ LICENSE Makefile src/

install: eta
	install -D eta \
	        $(DESTDIR)$(prefix)/bin/eta
	install -m 664 -D docs/eta.1 \
	        $(DESTDIR)$(prefix)/share/man/man1/eta.1

uninstall:
	-rm -f $(DESTDIR)$(prefix)/bin/eta \
	       $(DESTDIR)$(prefix)/share/man/man1/eta.1

distclean: clean

clean:
	-@rm -rf $(OBJ) ./eta

.PHONY: clean srcdist install uninstall
