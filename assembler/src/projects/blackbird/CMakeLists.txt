
project(blackdird CXX)
message(${COMMON_LIBRARIES})
message("12345566")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM "${ZLIB_INCLUDE_DIRS}")

add_executable(blackbird main.cpp utils/options.cpp utils/pipeline.cpp ../mts/contig_abundance.cpp)
target_link_libraries(blackbird dl mph_index utils BamTools input minimap2 ${COMMON_LIBRARIES} ${ZLIB_LIBRARIES})
#target_link_libraries(blackbird spades-stages graphio common_modules BamTools minimap2 ${COMMON_LIBRARIES})


install(TARGETS blackbird
        DESTINATION bin
        COMPONENT runtime)
