CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17 -O2 -g3
DEBUG_FLAGS = -g3 -O0 -fsanitize=address

OBJ = epoll_server.o
BIN_ET = epoll_server_et
BIN_LT = epoll_server_lt

all: $(BIN_LT) $(BIN_ET)

$(BIN_ET): CPPFLAGS = -DEPOLL_MODE_ET
$(BIN_ET): $(OBJ)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_LT): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# Debug mode is for Edge Trigger mode by default
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN_ET) $(BIN_LT) $(OBJ) debug

.PHONY: all clean
