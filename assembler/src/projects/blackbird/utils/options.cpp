//
// Created by Dima on 7/23/19.
//
#include "options.h"

std::string OptionBase::bam = "";
std::string OptionBase::reference = "";
std::string OptionBase::output_folder = "";
std::string OptionBase::region_file = "";
int OptionBase::threads = 1;
bool OptionBase::verbose = false;
std::string OptionBase::path_to_spades = "spades.py";