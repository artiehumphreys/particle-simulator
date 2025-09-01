#!/bin/bash
APP=build/bin/ParticleSim

cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-fno-omit-frame-pointer"
cmake --build build -j

"$APP" & pid=$!
sleep 1
sample $pid 15 -file sample4.txt
kill $pid
