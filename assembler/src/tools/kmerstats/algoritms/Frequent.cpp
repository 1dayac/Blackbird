//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#include "Frequent.h"
#include "../AdditionalFunction.h"
#include <map>


void WrapperFrequent(char *p, off_t size, int kmer, double k = 0.0, double l = 0.0) {
    Frequent(p, size, 40);
}

void Frequent(char *p, off_t size, int k) {
    int n = 0;
    std::map<char, int> m_count;
    for (off_t len = 0; len < size; ++len) {
        n++;
        if (m_count.find(p[len]) == m_count.end()) {
            ++m_count[ p[len] ];
        } else if (m_count.size() < k - 1) {
            m_count[ p[len] ] = 1;
        } else {
            std::map<char,int>::iterator it;
            for ( it = m_count.begin() ; it != m_count.end(); ++it ) {
                --(*it).second;
                if ((*it).second == 0) {
                    m_count.erase(it);
                }
            }
        }
    }

    createDatFile(m_count);
}



