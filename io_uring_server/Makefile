CC = gcc
CFLAGS = -Wall -O2
DEBUG_FLAGS = -Wall -g3 -O0 -fsanitize=address
LDFLAGS = -luring

OBJ = io_uring_server.o
BIN = io_uring_server

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

debug: CFLAGS = $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(OBJ)

.PHONY: all clean
