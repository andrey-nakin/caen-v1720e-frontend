cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(CxxReadoutExample VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "CAEN V1720e Readout Example Application")
set (CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME cxx-readout-example)
target_link_libraries(${PROJECT_NAME} CAENDigitizer)
target_link_libraries(${PROJECT_NAME} CaenLib)

#install(TARGETS ${PROJECT_NAME} DESTINATION bin)
