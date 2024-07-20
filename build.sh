#!/bin/bash

set -eaux

export STARTING_DIR="$(pwd)"
export INSTALL_PREFIX="$HOME/.local"
export BUILD_DIR="cmake-build-release"
export CXX="clang++"

function cleanup {
  cd $STARTING_DIR
}
trap cleanup EXIT

cmake -S . -B $BUILD_DIR
cmake --build $BUILD_DIR --config Release
cd $BUILD_DIR
ctest -V

cd ..

cmake --install $BUILD_DIR --prefix "$INSTALL_PREFIX" --config Release
