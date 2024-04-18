#!/bin/bash

# TESTED on Debian GNU/Linux 12 (bookworm) aarch64

set -e

rm -rf ./build
mkdir -p ./build
mkdir -p ./release

git submodule init
git submodule update
./tools/cleanup.sh
./tools/autogen.sh
./configure --prefix=$(realpath ./build) --enable-static --disable-shared
make -j8
make install

progname=main_$(uname --kernel-name )_$(uname --processor)

gcc -Wall -Werror -O3 -static \
	-I./build/include -L./build/lib \
	./main.c -o ./release/$progname -lwallycore -lsecp256k1
strip ./release/$progname
echo "build successfully!"
./release/$progname
