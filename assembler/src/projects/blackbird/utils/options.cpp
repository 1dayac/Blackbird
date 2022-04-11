//
// Created by Dima on 7/23/19.
//
#include "options.h"

std::string OptionBase::bam = "";
std::string OptionBase::long_read_bam = "";
std::string OptionBase::long_read_fastq = "";
std::string OptionBase::reference = "";
std::string OptionBase::output_folder = "";
std::string OptionBase::region_file = "";
int OptionBase::threads = 1;
bool OptionBase::verbose = false;
bool OptionBase::keep_assembly_folders = false;
bool OptionBase::dont_collect_reads = false;
bool OptionBase::use_long_reads = false;
std::string OptionBase::path_to_spades = "spades.py";
std::string OptionBase::cmd_line = "";