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
                "           bam          b      Position-sorted and indexed bam-file with BX tags\n"
                "           rerefence    r      BWA-indexed reference genome.\n"
                "           output       o      Folder where results will be stored.\n"
                "           spades       s      Path to spades.py (not needed if spades.py is in path).\n"
                "           threads      t      Number of threads - more the better.\n"
                "           regions      r      File with chromosomes and coordinates.\n"
                "           verbose      v      Print additional output (for debug purposes).\n"
                "           help         h      Print this message.\n"
                "\nReport bugs to dmm2017@med.cornell.edu \n\n";




static const char* shortopts = "b:s:r:o:g:t:hvnd";
static const struct option longopts[] = {
        {"bam",        required_argument, 0, 'b' },
        {"reference",  required_argument, 0, 'r' },
        {"regions",    optional_argument, 0, 'g' },
        {"output",     required_argument, 0, 'o' },
        {"verbose",    no_argument,       0, 'v' },
        {"delete",     no_argument,       0, 'd' },
        {"no-collect", no_argument,       0, 'n' },
        {"spades",     optional_argument, 0, 's' },
        {"threads",    optional_argument, 0, 't' },
        {"help",       no_argument,       0, 'h' },
        {0,            0,                 0,  0  }
};


bool getOptions(int argc, char **argv) {

    bool help = false;
    for (char c; (c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1;) {
        std::istringstream arg(optarg != NULL ? optarg : "");
        switch (c) {
            case 'b' : arg >> OptionBase::bam; break;
            case 'd' : OptionBase::keep_assembly_folders = true; break;
            case 'r' : arg >> OptionBase::reference; break;
            case 'g' : arg >> OptionBase::region_file; break;
            case 't' : arg >> OptionBase::threads; break;
            case 'o' : arg >> OptionBase::output_folder; break;
            case 's' : arg >> OptionBase::path_to_spades; break;
            case 'v' : OptionBase::verbose = true; break;
            case 'n' : OptionBase::dont_collect_reads = true; break;
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
        std::string cmdLine = "";
        std::cout << "Command line: ";
        for (int i = 0; i < argc; ++i) {
            std::cout << argv[i] << " ";
            cmdLine += argv[i];
            cmdLine += " ";
        }
        std::cout << std::endl;
        OptionBase::cmd_line = cmdLine;
        runBlackbird();
    }
    return 0;
}