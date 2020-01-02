# Install script for directory: /Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/pyyaml3" TYPE FILE FILES
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/__init__.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/composer.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/constructor.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/cyaml.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/dumper.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/emitter.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/error.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/events.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/loader.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/nodes.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/parser.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/reader.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/representer.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/resolver.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/scanner.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/serializer.py"
    "/Users/dima/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml3/tokens.py"
    )
endif()

