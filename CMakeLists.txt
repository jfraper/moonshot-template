cmake_minimum_required(VERSION 3.14)

# Change "mygame" to your project name
project(mygame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# Engine
set(ENGINE_DIR ${CMAKE_SOURCE_DIR}/engine)
if(NOT EXISTS ${ENGINE_DIR}/CMakeLists.txt)
  message(FATAL_ERROR "Engine not found at ${ENGINE_DIR}. Run: git submodule update --init --recursive")
endif()

if(NOT TARGET moonshot)
  add_subdirectory(${ENGINE_DIR} ${CMAKE_BINARY_DIR}/moonshot)
endif()

# Platform detection
if(APPLE)
  set(PLATFORM "macos")
elseif(WIN32)
  set(PLATFORM "windows")
else()
  set(PLATFORM "linux")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
set(GAME_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/dist/${BUILD_TYPE_LOWER}-${PLATFORM})

# Game sources (auto-discovered, recursive)
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
  "src/*.cpp" "src/*.h"
)

include(${ENGINE_DIR}/cmake/moonshot_game.cmake)
moonshot_add_game(${PROJECT_NAME}
  SOURCES ${GAME_SOURCES}
  OUTPUT_DIR ${GAME_OUTPUT_DIR}
)

# Allow includes relative to src/
if(TARGET ${PROJECT_NAME}_game)
  target_include_directories(${PROJECT_NAME}_game PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()
if(TARGET ${PROJECT_NAME} AND NOT TARGET ${PROJECT_NAME}_game)
  target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()
