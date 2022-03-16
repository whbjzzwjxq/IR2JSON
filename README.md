# IR2JSON

## Description

    A LLVM Pass to extract information from llvm to json

## Build

    Assume your llvm-project locates ./llvm-project
    git clone ./llvm-project/llvm/lib/Transforms/IR2JSON
    add ```add_subdirectory(IR2JSON)``` to ./llvm-project/llvm/lib/Transforms/CMakeLists.txt
    cd ./llvm-project/build
    cmake --build .

## Usage

    ```opt -load path-to-lib/IR2JSON.so --ir2json input.bc 1> /dev/null 2> output.json```
