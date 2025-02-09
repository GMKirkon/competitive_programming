#!/bin/bash
cmake --preset Release -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build cmake-build-Release/ --target tests -- -j 8
./cmake-build-Release/tests 