CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17
DEBUG_FLAGS = -g3 -O0 -fsanitize=address
LD_DEBUG_FLAGS = -fsanitize=address

OBJ = server.o
BIN = myserver

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += $(LD_DEBUG_FLAGS)
debug: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) $(OBJ)

.PHONY: all clean
