#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(UtilLibTest VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Utility Library Tests")
set (CMAKE_CXX_STANDARD 14)

include_directories($ENV{MIDASSYS}/include)
include(FindGTest)
find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include/)

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.hxx)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME util-lib-test)

target_link_libraries(${PROJECT_NAME} GTest::GTest GTest::Main)
target_link_libraries(${PROJECT_NAME} UtilLib)

add_test(
  NAME ${PROJECT_NAME}
  COMMAND ${PROJECT_NAME}
)


