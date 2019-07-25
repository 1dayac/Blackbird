//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#include "CountMinSketch.h"
#include <cmath>

void WrapperCountMinSketch(char *p, off_t size, int kmer, double k = 0.0, double l = 0.0) {
    CountMinSketch(p, size, 0.3, 0.95, kmer);
}

void CountMinSketch(char *p, off_t size, double eps, double sigma, int kmer) {
    int w = 2.718281/eps + 1;
    int d = log(1 / sigma) + 1;

    int **m_count = new int *[d];
    for (int i = 0; i < d; i++){
        m_count[i] = new int [w];
    }
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < w; ++j) {
            m_count[i][j] = 0;
        }
    }
    int count = 0;
    for (off_t len = 0; len < size; ++len) {
        if (count == kmer) {
            for (int j = 0; j < d; j++) {
                int r = p[len] % w;
                m_count[j][r] += 1;
            }
            --count;
        }
        ++count;
   }

    int t = 'A' % w;
    int min = m_count[0][t];
    for (int i = 0; i < d; ++i) {
        if (min < m_count[i][t]) {
            min = m_count[i][t];
        }
    }

    std::cout << min << std::endl;

    for (int i = 0; i < d; i++) {
        delete[] m_count[i];
    }
    delete[] m_count;
}

