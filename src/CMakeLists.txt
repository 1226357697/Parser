
set(
    PARSER_SRC_LIST
    BinaryTypes.h
    BinaryModule.h
    BinaryModule.cpp
    PEBinaryModule.h
    PEBinaryModule.cpp
    Parser.h
    Parser.cpp
    Instruction.h
    Instruction.cpp
    BasicBlock.h
    BasicBlock.cpp
    Function.h
    Function.cpp
    Architecture.h
    Architecture.cpp
    Disassembler.h
    Disassembler.cpp
    CapstoneDisassembler.h
    CapstoneDisassembler.cpp
    X86Architecture.h
    X86Architecture.cpp
    InstructionAnalyzer.h
    InstructionAnalyzer.cpp
    X86InstructionAnalyzer.h
    X86InstructionAnalyzer.cpp
    CrossReference.h
    CrossReference.cpp
    Operand.h
    Operand.cpp
    OperandDecoder.h
    OperandDecoder.cpp
    X86OperandDecoder.h
    X86OperandDecoder.cpp
    main.cpp
)

add_executable(Parser ${PARSER_SRC_LIST})

target_link_libraries(Parser PRIVATE capstone LIB_LIEF)

set_target_properties(Parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)