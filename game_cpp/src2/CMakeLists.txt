cmake_minimum_required(VERSION 3.16)
project(behavior_driven_cells_headless)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(behavior_driven_cells_headless
  main.cpp
  Vector.cpp
  myPoint.cpp
  myMatrix.cpp
  particle.cpp
  land.cpp
  food.cpp
  barrier.cpp
  cell.cpp
  world.cpp
)

find_path(BOOST_INCLUDE_DIR boost/beast/core.hpp
  HINTS
    /opt/homebrew/include
    /opt/homebrew/opt/boost/include
)

if(NOT BOOST_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find Boost headers. Install with: brew install boost")
endif()

add_executable(behavior_driven_cells_ws
  server.cpp
  snapshot_json.cpp
  Vector.cpp
  myPoint.cpp
  myMatrix.cpp
  particle.cpp
  land.cpp
  food.cpp
  barrier.cpp
  cell.cpp
  world.cpp
)

target_include_directories(behavior_driven_cells_ws PRIVATE ${BOOST_INCLUDE_DIR})
target_compile_definitions(behavior_driven_cells_ws PRIVATE BOOST_ERROR_CODE_HEADER_ONLY)
target_link_libraries(behavior_driven_cells_ws PRIVATE Threads::Threads)
