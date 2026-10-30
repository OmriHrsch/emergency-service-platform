CXX ?= g++

TARGET := bin/StompEMIClient
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,bin/%.o,$(SOURCES))

CPPFLAGS := -Iinclude
CXXFLAGS := -g -Wall -Wextra -std=c++11 -pthread
LDLIBS := -pthread

ifeq ($(shell uname -s),Darwin)
BOOST_PREFIX ?= $(shell brew --prefix boost 2>/dev/null)
ifneq ($(BOOST_PREFIX),)
CPPFLAGS += -I$(BOOST_PREFIX)/include
LDFLAGS += -L$(BOOST_PREFIX)/lib
endif
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/%.o: src/%.cpp | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all run clean
