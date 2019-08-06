//
// Created by Dima on 7/23/19.
//
#include <sstream>

#ifndef BLACKBIRD_OPTIONS_H
#define BLACKBIRD_OPTIONS_H

class OptionBase {
public:
    static std::string bam; // the bam to analyze
    static std::string reference; // file with list of tags to be extracted
    static std::string output_folder;
    static int mapping_quality;
    static bool verbose;
    static std::string path_to_spades;
};

#endif //BLACKBIRD_OPTIONS_H
