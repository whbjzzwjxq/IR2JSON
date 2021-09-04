add_llvm_library( LLVMIR2JSON MODULE
    ir2json.cpp

    PLUGIN_TOOL
    opt
)