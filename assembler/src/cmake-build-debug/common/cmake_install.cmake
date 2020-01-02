# Install script for directory: /Users/dima/Desktop/Blackbird/assembler/src/common

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/pipeline/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/sequence/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/assembly_graph/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/modules/path_extend/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/modules/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/paired_info/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/stages/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/utils/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/io/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/utils/kmer_mph/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/common/modules/coverage_model/cmake_install.cmake")

endif()

