CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test demo clean
all: build/warmdebt
build:
	mkdir -p build
build/warmdebt: main.cpp warmdebt.hpp | build
	$(CXX) $(CXXFLAGS) main.cpp -o $@
build/tests: tests.cpp warmdebt.hpp | build
	$(CXX) $(CXXFLAGS) tests.cpp -o $@
test: build/tests build/warmdebt
	./build/tests
	./build/warmdebt demo > /dev/null
demo: build/warmdebt
	./build/warmdebt demo
clean:
	rm -rf build
