.PHONY: all build install clean format

all: build

build:
	cmake -B build -G Ninja
	cmake --build build --target voy

install: build
	sudo cp ./build/voy /usr/local/bin

clean:
	rm -rf build

format:
	find include src test -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec clang-format -style=file -i {} +
