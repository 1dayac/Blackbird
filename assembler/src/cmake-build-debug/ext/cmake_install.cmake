# Install script for directory: /Users/dima/Desktop/Blackbird/assembler/ext/src

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
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/jemalloc/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/nlopt/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/python_libs/joblib2/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/python_libs/joblib3/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/python_libs/pyyaml2/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/python_libs/pyyaml3/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/ConsensusCore/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/bamtools/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/samtools/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/cppformat/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/ssw/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/cityhash/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/getopt_pp/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/llvm/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/htrie/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/bwa/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/gqf/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/edlib/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/minimap2/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird/assembler/src/cmake-build-debug/ext/gfa1/cmake_install.cmake")

endif()

