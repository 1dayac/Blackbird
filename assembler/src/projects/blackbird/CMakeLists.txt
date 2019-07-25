
project(blackdird CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM "${ZLIB_INCLUDE_DIRS}")

add_executable(blackbird main.cpp utils/options.cpp utils/pipeline.cpp ../mts/contig_abundance.cpp)

target_link_libraries(blackbird spades-stages graphio common_modules BamTools ${COMMON_LIBRARIES})


install(TARGETS blackbird
        DESTINATION bin
        COMPONENT runtime)
