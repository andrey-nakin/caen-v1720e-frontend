cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(R VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "R packages and scripts")

add_subdirectory(packages)
add_subdirectory(scripts)
