CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17 -O2
DEBUG_FLAGS = -g3 -O0 -fsanitize=address

SRC = epoll_server.cc
BIN_ET = epoll_server_et
BIN_LT = epoll_server_lt

all: $(BIN_LT) $(BIN_ET)

$(BIN_ET): CPPFLAGS = -DEPOLL_MODE_ET
$(BIN_ET): $(SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_LT): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Debug mode is for Edge Trigger mode by default
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(BIN_ET) $(BIN_LT) epoll_server.o  debug

.PHONY: all clean
