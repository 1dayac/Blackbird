# CMake generated Testfile for 
# Source directory: /home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/stress
# Build directory: /home/dmm2017/Desktop/algorithmic-biology/assembler/src/cmake-build-debug/ext/jemalloc/test/stress
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(hookbench "/home/dmm2017/Desktop/algorithmic-biology/assembler/src/cmake-build-debug/bin/hookbench")
set_tests_properties(hookbench PROPERTIES  _BACKTRACE_TRIPLES "/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/CMakeLists.txt;30;add_test;/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/stress/CMakeLists.txt;13;createTest;/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/stress/CMakeLists.txt;0;")
add_test(microbench "/home/dmm2017/Desktop/algorithmic-biology/assembler/src/cmake-build-debug/bin/microbench")
set_tests_properties(microbench PROPERTIES  _BACKTRACE_TRIPLES "/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/CMakeLists.txt;30;add_test;/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/stress/CMakeLists.txt;13;createTest;/home/dmm2017/Desktop/algorithmic-biology/assembler/ext/src/jemalloc/test/stress/CMakeLists.txt;0;")
