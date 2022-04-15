# IR2JSON

## Description

A LLVM Pass to extract information from llvm to json, using old pass manager of LLVM.

## Build

Assume your llvm-project locates ./llvm-project

    git clone git@github.com:whbjzzwjxq/Re-IR2JSON.git ./llvm-project/llvm/lib/Transforms/IR2JSON

add  `add_subdirectory(IR2JSON)` to ./llvm-project/llvm/lib/Transforms/CMakeLists.txt

    cd ./llvm-project/build
    cmake --build .

## Usage

    opt -load -enable-new-pm=0 path-to-lib/IR2JSON.so --IR2JSON input.bc 1> /dev/null 2> output.json

## Compilation of LLVM

Do NOT compile the LLVM with this pass, instead, compile this pass after LLVM compiled.

    cmake -S llvm -B build -G Ninja \
    -DLLVM_ENABLE_PROJECTS='clang;lld' \
    -DLLVM_TARGETS_TO_BUILD="X86;ARM;RISCV" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DLLVM_PARALLEL_LINK_JOBS=1 \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
