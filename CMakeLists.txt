cmake_minimum_required(VERSION 3.1)
project(OpenABL)

set(OpenABL_SOURCES
		src/main.cpp)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser src/Parser.y ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
FLEX_TARGET(Lexer src/Lexer.l ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

set(CMAKE_CXX_STANDARD 11)
include_directories(src/ ${CMAKE_CURRENT_BINARY_DIR})
add_executable(OpenABL
		${OpenABL_SOURCES}
		${BISON_Parser_OUTPUTS}
		${FLEX_Lexer_OUTPUTS}
		${OpenABL_SOURCES})
