//
// Created by Dima on 7/24/19.
//

#include <iostream>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include "utils/pipeline.h"
#include "utils/options.h"
#define AUTHOR "Dmitry Meleshko <dmm2017@med.cornell.edu>"

#define VERSION "0.0.1"

static const char *BLACKBIRD_USAGE_MESSAGE =
        "------------------------------------------------------------\n"
                "-------- Blackbird - 10X indel detection by assembly -------\n"
                "------------------------------------------------------------\n"
                "Program: Blackbird \n"
                "Version: " VERSION "\n"
                "Contact: Dmitry Meleshko [ dmm2017@med.cornell.edu ]\n"
                "Usage: blackbird [options]\n\n"
                "Options:\n"
                "           bam            Position-sorted and indexed bam-file with BX tags\n"
                "           rerefence      BWA-indexed reference genome.\n"
                "           output_folder  Folder where results will be stored.\n"
                "           spades         Path to spades.py (not needed if spades.py is in path).\n"
                "           verbose        Print additional output (for debug purposes).\n"
                "           help           Print this message.\n"
                "\nReport bugs to dmm2017@med.cornell.edu \n\n";




static const char* shortopts = "b:s:r:o:hv";
static const struct option longopts[] = {
        {"bam",        required_argument, 0, 'b' },
        {"reference",  required_argument, 0, 'r' },
        {"output",     required_argument, 0, 'o' },
        {"verbose",    no_argument,       0, 'v' },
        {"spades",     optional_argument, 0, 's' },
        {"help",       no_argument,       0, 'h' },
        {0,            0,                 0,  0  }
};


bool getOptions(int argc, char **argv) {

    bool help = false;
    for (char c; (c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1;) {
        std::istringstream arg(optarg != NULL ? optarg : "");
        switch (c) {
            case 'b' : arg >> OptionBase::bam; break;
            case 'r' : arg >> OptionBase::reference; break;
            case 'o' : arg >> OptionBase::output_folder; break;
            case 's' : arg >> OptionBase::path_to_spades; break;
            case 'v' : OptionBase::verbose = true; break;
            case 'h' : help = true; break;
        }
    }

    if (OptionBase::bam == "" || OptionBase::reference == "" || OptionBase::output_folder == "") {
        return 0;
    }
    return !help;
}

void runBlackbird() {
    BlackBirdLauncher launcher;
    launcher.Launch();
}

int main(int argc, char **argv) {

    if (!getOptions(argc, argv)) {
        std::cerr << BLACKBIRD_USAGE_MESSAGE << std::endl;
        return 1;
    } else {
        runBlackbird();
    }
    return 0;
}