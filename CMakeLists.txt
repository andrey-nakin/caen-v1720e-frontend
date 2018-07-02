cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GaemV1720eFrontend VERSION 1.0.0 LANGUAGES C DESCRIPTION "MIDAS Frontend for CAEN V1720E Digitizer")

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME v1720e-frontend)
target_link_libraries(${PROJECT_NAME} CAENDigitizer)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
