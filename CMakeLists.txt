cmake_minimum_required(VERSION 2.6)
project(ish)

list(APPEND LIBS)
set(CMAKE_C_FLAGS "-O2 -Wall -fstack-protector")

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

execute_process(COMMAND echo "USE_READLINE ="${USE_READLINE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

if(USE_READLINE)
  include_directories(${READLINE_INCLUDE})
  add_definitions(-DUSE_READLINE=1)
  list(APPEND LIBS ${READLINE_LIB} ncurses)
endif()

add_executable(ish ish.c parser/parse.c getline.c int_list.c job.c path.c)
target_link_libraries(ish ${LIBS})
