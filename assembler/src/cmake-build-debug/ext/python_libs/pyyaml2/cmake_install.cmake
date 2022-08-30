# Install script for directory: /home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2

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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/pyyaml2" TYPE FILE FILES
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/__init__.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/composer.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/constructor.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/cyaml.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/dumper.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/emitter.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/error.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/events.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/loader.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/nodes.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/parser.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/reader.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/representer.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/resolver.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/scanner.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/serializer.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/pyyaml2/tokens.py"
    )
endif()

