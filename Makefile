.PHONY: all build install test clean format

all: clean install

build-common:
	cmake -B build -G Ninja

build: build-common
	cmake --build build --target voy
	
install: build
	sudo cp ./build/voy /usr/local/bin

test: build-common
	cmake --build build --target voy_tests
	./build/voy_tests

clean:
	rm -rf build

format:
	find include src test -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec clang-format -style=file -i {} +
