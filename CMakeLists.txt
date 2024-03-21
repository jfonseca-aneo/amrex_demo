cmake_minimum_required(VERSION 3.20.0)

project(amrex_demo LANGUAGES C CXX)

set (AMREX_DEMO_BUGS    jfonseca@aneo.fr)
set (AMREX_DEMO_SRCDIR  "${PROJECT_SOURCE_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 17)

include(CheckCXXCompilerFlag)

if ($(AMREX_DEMO_SOURCE_DIR) STREQUAL $(CMAKE_BINARY_DIR))
  message(FATAL_ERROR "In place build not allowed; please use a separate build directory")
endif()

find_package(Git)

include(GetVersionFromGitTag)

set(amrex_demo VERSION ${amrex_demo_VERSION_STRING})

message(STATUS "Configuring version : ${amrex_demo_VERSION_STRING_FULL}")

add_library(AMREX_DEMO SHARED
  src/AmrMeshFromTiff.cpp
)

target_include_directories(AMREX_DEMO
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_compile_features(AMREX_DEMO PRIVATE cxx_std_17)

find_package(MPI REQUIRED)
find_package(TIFF REQUIRED)
find_package(AMReX REQUIRED)

include_directories(${AMREX_INCLUDE_DIRS})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_OPTIONS}")
set (CMAKE_CXX_COMPILER "${MPI_CXX_COMPILER}")

target_link_libraries(AMREX_DEMO
  PRIVATE AMReX::amrex TIFF::TIFF
  PUBLIC AMReX::amrex
)

add_subdirectory (demo_exe)

install (TARGETS AMREX_DEMO DESTINATION lib)

add_custom_target(copy-files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/test
  ${CMAKE_CURRENT_BINARY_DIR}/demo_exe)