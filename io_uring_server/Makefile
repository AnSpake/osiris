CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=gnu17 -O2
DEBUG_FLAGS = -Wall -g3 -O0 -fsanitize=address
LDFLAGS = -luring

SRC = io_uring_server.c
BIN = io_uring_server
BIN_SQPOLL = io_uring_server_sqpoll

SRC_no_abs = io_uring_server_no_abs.c
BIN_no_abs = io_uring_server_no_abs

all: $(BIN_SQPOLL) $(BIN) $(BIN_no_abs)

$(BIN_SQPOLL): CPPFLAGS = -DKERNEL_POLL_MODE
$(BIN_SQPOLL): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_no_abs): $(SRC_no_abs)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Debug only for no automatic buffer selection version
debug: CFLAGS = $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(SRC_no_abs)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(BIN_SQPOLL) $(BIN_no_abs) io_uring_server.o debug

.PHONY: all clean $(BIN_SQPOLL)
