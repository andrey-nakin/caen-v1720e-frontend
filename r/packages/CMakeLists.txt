cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(RPackages VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "R packages")

set(GNEIS_DAQ_SOURCES ${PROJECT_SOURCE_DIR}/gneis.daq)

file(COPY 
	${GNEIS_DAQ_SOURCES} 
	DESTINATION ${PROJECT_BINARY_DIR}
	PATTERN ".*" EXCLUDE
	PATTERN "*.Rproj" EXCLUDE
	)

add_custom_command(
	OUTPUT gneis.daq.tar.gz
	COMMAND ${CMAKE_COMMAND} -E tar cfvz gneis.daq.tar.gz gneis.daq
	)
	
add_custom_target(
	GneisDaqPackage ALL
	DEPENDS gneis.daq.tar.gz 
	)

if(DEFINED R_PACKAGES_INSTALL_DIR)
  install(DIRECTORY scripts DESTINATION ${R_PACKAGES_INSTALL_DIR})
endif(DEFINED R_PACKAGES_INSTALL_DIR)
