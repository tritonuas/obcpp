.PHONY: build setup run clean
build:
	cd build && make

run:
	bin/obcpp

setup: CMakeLists.txt
	mkdir -p build && cd build && cmake .. && make