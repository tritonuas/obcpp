#!/bin/sh

# This script will be run from inside the build directory because it is executed from 
# CMake, which we run from inside the build directory
if [ "${GITHUB_ACTIONS}" != "true" ]; then
    git submodule update --init --remote
fi

# Compile both of the protobuf files into the build directory
mkdir gen_protos
cd gen_protos
mkdir protos
cd ..
protoc -I=../protos --cpp_out=./gen_protos/protos ../protos/obc.proto