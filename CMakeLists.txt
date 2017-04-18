cmake_minimum_required(VERSION 3.1)
project(OpenABL)

set(OpenABL_SOURCES
    src/ParserContext.cpp
    src/AST.cpp
    src/Analysis.cpp
    src/AnalysisVisitor.cpp
    src/main.cpp
    src/backend/CBackend.cpp
    src/backend/CPrinter.cpp
    src/backend/FlameGPUBackend.cpp)

find_package(BISON 3.0)
find_package(FLEX 2.5)

BISON_TARGET(Parser src/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
FLEX_TARGET(Lexer src/Lexer.l ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare")
endif()

add_custom_target(copy-assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/asset ${CMAKE_BINARY_DIR}/asset)

include_directories(src/ ${CMAKE_CURRENT_BINARY_DIR})
add_executable(OpenABL
    ${OpenABL_SOURCES}
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
    ${OpenABL_SOURCES})
