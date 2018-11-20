cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(FrontEndV1720 VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "MIDAS Frontend for CAEN V1720 ")
set (CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.hxx)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME fe_v1720)
target_link_libraries(${PROJECT_NAME} CAENDigitizer)
target_link_libraries(${PROJECT_NAME} CaenLib)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
