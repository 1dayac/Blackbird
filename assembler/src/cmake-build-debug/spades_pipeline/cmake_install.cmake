# Install script for directory: /Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline" TYPE FILE FILES
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/run_contig_breaker.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/commands_parser.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/process_cfg.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/support.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/options_storage.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/lucigen_nxmate.py"
    "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/options_parser.py"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline/" TYPE DIRECTORY FILES "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/scripts" REGEX "/[^/]*\\.pyc$" EXCLUDE)
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline/" TYPE DIRECTORY FILES "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/stages" REGEX "/[^/]*\\.pyc$" EXCLUDE)
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline/" TYPE DIRECTORY FILES "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/truspades" REGEX "/[^/]*\\.pyc$" EXCLUDE)
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline/" TYPE DIRECTORY FILES "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/common" REGEX "/[^/]*\\.pyc$" EXCLUDE)
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spades/spades_pipeline/" TYPE DIRECTORY FILES "/Users/dima/Desktop/Blackbird2/assembler/src/spades_pipeline/executors" REGEX "/[^/]*\\.pyc$" EXCLUDE)
endif()

