#!/bin/sh
cmake -B build -S .
if [ $? -eq 0 ]; then
    ln -s build/compile_commands.json compile_commands.json
    cmake --build build
fi
if [ $? -eq 0 ]; then
    ./build/AudioVisualizer
fi

