cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(ReadoutExample VERSION 1.0.0 LANGUAGES C DESCRIPTION "CAEN V1720e Readout Example Application")

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME readout-example)
target_link_libraries(${PROJECT_NAME} CAENDigitizer)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
