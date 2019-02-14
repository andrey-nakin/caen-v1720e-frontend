cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(CaenV1720eFrontend VERSION 1.0.0 LANGUAGES CXX C DESCRIPTION "MIDAS Frontend for CAEN V1720E Digitizer")

option(BUILD_TESTS "Enable unit test building" OFF)
option(BUILD_EXAMPLE "Enable example building" ON)
option(BUILD_DOC "Enable documentation building" OFF)

add_subdirectory(caen-lib)
add_subdirectory(midas-util-lib)
add_subdirectory(util-lib)
add_subdirectory(analyzer-lib)
add_subdirectory(math-lib)
add_subdirectory(fe-v1720)
add_subdirectory(an-basic-waveform)

if(BUILD_EXAMPLE)
  add_subdirectory(fe-sinus)
  add_subdirectory(readout-example)
  add_subdirectory(cxx-readout-example)
endif()

if(BUILD_TESTS)
  add_subdirectory(math-lib-test)
endif()

if(BUILD_DOC)
  add_subdirectory(doc)
endif()
