//
// Created by Dima on 7/23/19.
//
#include <sstream>

#ifndef BLACKBIRD_OPTIONS_H
#define BLACKBIRD_OPTIONS_H

class OptionBase {
public:
    static std::string bam; // the bam to analyze
    static std::string long_read_bam; // long-read bam
    static std::string long_read_fastq; // long read fastq-file
    static std::string reference; // file with list of tags to be extracted
    static std::string region_file; // call regions from file
    static std::string output_folder;
    static bool dont_collect_reads;
    static bool use_long_reads;
    static int threads;
    static bool verbose;
    static bool keep_assembly_folders;
    static std::string path_to_spades;
    static std::string cmd_line;
};

#endif //BLACKBIRD_OPTIONS_H
