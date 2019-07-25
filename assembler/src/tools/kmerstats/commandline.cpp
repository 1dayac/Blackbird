//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#include "commandline.h"

#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "analyses.h"

CommandLine::CommandLine(char **argv, int argc): isAlg(false) {
    int c;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"file", 1, 0, 0},
            {"kmer", 1, 0, 0},
            {"help", 0, 0, 0},
            {"algoritm", 1, 0, 0},
            {"merge", 0, 0, 0},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "f:k:a:h",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            if (long_options[option_index].name == "file") {
                m_line = optarg;
            } else if (long_options[option_index].name == "kmer") {
                m_kmer = optarg;
            } else if (long_options[option_index].name == "algoritm") {
                m_nameAlg = optarg;
                isAlg = true;
            }
            break;

        case 'f':
            m_line = optarg;
            break;

        case 'k':
            m_kmer = optarg;
            break;

        case 'a':
            m_nameAlg = optarg;
            isAlg = true;
            break;

        case 'h':
        default:
            std::cout << "Help: ./kmerstat [-f|--file] <filename> [-k|--kmer] <kmer> [-a|--algoritm] <algoritm name>" << std::endl;
        }
    }

    Analyses a(m_line, m_kmer, m_nameAlg, isAlg);
}

