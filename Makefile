CC=gcc
SRC := src
OBJ := obj

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

eta: $(OBJECTS)
	$(CC) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) -I$(SRC) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJ) ./eta
