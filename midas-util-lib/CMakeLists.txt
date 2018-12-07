#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(MidasUtilLib VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Midas Utility Library")
set (CMAKE_CXX_STANDARD 14)
include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories($ENV{MIDASSYS}/include)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.hxx)

#----------------------------------------------------------------------------
# Add and configure the library
#
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME caen)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

#----------------------------------------------------------------------------
# Install the library
#
#install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
#install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)
#export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

