cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GneisDaq VERSION 0.9.0 LANGUAGES CXX C DESCRIPTION "GNEIS DAQ")

set(
	GNEIS_DAQ_VERSION
	${PROJECT_VERSION}
)

option(BUILD_ANALYZERS "Enable example building" ON)
option(BUILD_FRONTENDS "Enable example building" ON)
option(BUILD_EXAMPLES "Enable example building" ON)
option(BUILD_UTILS "Enable utility building" ON)
option(BUILD_TESTS "Enable unit test building" OFF)
option(BUILD_DOCS "Enable documentation building" OFF)

add_subdirectory(midas-util-lib)
add_subdirectory(caen-lib)
add_subdirectory(util-lib)
add_subdirectory(math-lib)

if(BUILD_FRONTENDS)
  add_subdirectory(fe-v1720)
  add_subdirectory(fe-v1724)
endif()

if(BUILD_ANALYZERS)
  add_subdirectory(analyzer-lib)
  add_subdirectory(an-basic-waveform)
endif()

if(BUILD_UTILS)
  add_subdirectory(gd-convert)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(fe-sinus)
  add_subdirectory(readout-example)
  add_subdirectory(cxx-readout-example)
endif()

if(BUILD_TESTS)
  add_subdirectory(util-lib-test)
  add_subdirectory(math-lib-test)
endif()

if(BUILD_DOCS)
  add_subdirectory(doc)
endif()
