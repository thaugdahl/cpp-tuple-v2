.PHONY: clean FORCE

CXX?=clang++
CXXFLAGS+=-std=c++11 -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wconversion -Wsign-conversion -Wfloat-equal -Wmissing-declarations -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wuninitialized -Wunused -Wdeprecated -Werror -O0 -g

main: main.cpp
	$(CXX) $(CXXFLAGS) $+ -o $@

clean:
	-rm *.o
	-rm main
