# Install script for directory: /home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/joblib2" TYPE FILE FILES
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/__init__.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/disk.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/format_stack.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/func_inspect.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/functools.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/hashing.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/logger.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/memory.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/my_exceptions.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/numpy_pickle.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/parallel.py"
    "/home/dmm2017/Desktop/Blackbird/assembler/ext/src/python_libs/joblib2/testing.py"
    )
endif()

