# Install script for directory: /Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/joblib2" TYPE FILE FILES
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/__init__.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/disk.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/format_stack.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/func_inspect.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/functools.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/hashing.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/logger.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/memory.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/my_exceptions.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/numpy_pickle.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/parallel.py"
    "/Users/dima/Desktop/Blackbird2/assembler/ext/src/python_libs/joblib2/testing.py"
    )
endif()

