CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

ifeq ($(shell uname),Darwin)
LIBS = -framework OpenGL -framework GLUT
else
LIBS = -lglut -lGLU -lGL
endif

mumbai_skate: src/main.cpp $(wildcard src/*.hpp)
	$(CXX) $(CXXFLAGS) src/main.cpp -o $@ $(LIBS)

run: mumbai_skate
	./mumbai_skate

test: mumbai_skate
	./mumbai_skate --selftest

clean:
	rm -f mumbai_skate

.PHONY: run test clean
