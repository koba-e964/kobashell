cmake_minimum_required(VERSION 2.6)
project(ish)

find_path(READLINE_ROOT
  NAMES include/readline/readline.h)
find_path(READLINE_INCLUDE
  NAMES readline/readline.h
  HINTS ${READLINE_ROOT}/include)
find_library(READLINE_LIB
  NAMES readline
  HINTS ${READLINE_ROOT}/lib)

if (READLINE_INCLUDE AND READLINE_LIB)
  set(USE_READLINE 1)
else()
  set(USE_READLINE 0)
endif()

execute_process(COMMAND echo "USE_READLINE="${USE_READLINE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)



add_executable(ish ish.c parser/parse.c parser/print.c)