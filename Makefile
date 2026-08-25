.PHONY: all build install test clean format

all: build install

build:
	cmake -B build -G Ninja

install: build
	cmake --build build --target voy
	sudo cp ./build/voy /usr/local/bin

test: build
	cmake --build build --target voy_tests
	./build/voy_tests

clean:
	rm -rf build

format:
	find include src test -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec clang-format -style=file -i {} +
