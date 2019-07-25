# Install script for directory: /Users/dima/Desktop/Blackbird2/assembler/src/projects

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
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/spades/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/hammer/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/ionhammer/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/corrector/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/scaffold_correction/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/kmercount/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/gbuilder/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/gmapper/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/edge_profiles/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/spaligner/cmake_install.cmake")
  include("/Users/dima/Desktop/Blackbird2/assembler/src/cmake-build-debug/projects/blackbird/cmake_install.cmake")

endif()

