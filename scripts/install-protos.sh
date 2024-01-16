#!/bin/sh

# This script will be run from inside the build directory because it is executed from 
# CMake, which we run from inside the build directory
if [ "$GITHUB_ACTIONS" != "true" ]
    git submodule update --init --remote
fi

# Compile both of the protobuf files into the build directory
mkdir gen_protos
protoc -I=../protos --cpp_out=./gen_protos ../protos/obc.proto