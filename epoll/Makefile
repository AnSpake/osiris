CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++17 -O2
DEBUG_FLAGS = -g3 -O0 -fsanitize=address

OBJ = epoll_server.o
BIN = epoll_server

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(OBJ)

.PHONY: all clean
