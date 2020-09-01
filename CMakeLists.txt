
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(
  engine_
  VERSION 1.0
  LANGUAGES CXX C)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
    "*.c"
)



add_executable(Engine_ ${SRC})
include_directories(include)
set_target_properties(Engine_
  PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
  )

