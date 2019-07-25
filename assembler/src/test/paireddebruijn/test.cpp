//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"
//#include "hashTableTest.hpp"
//#include "graphConstructionTest.hpp"
#include "graphioTest.hpp"
#include "pairThreadingTest.hpp"
#include "pairedGraphTest.hpp"

void runSuite() {
    cute::suite s;
    //TODO add your test here
//    s += HashTableSuite();
//    s += CheckStoreVertexSuite();
//    s += CheckUniqueWaySuite();
//    s += GoUniqueWaySuite();
//    s += GraphioSuite();
//    s += PairThreadingSuite();
    s += PairedGraphSuite();
    cute::ide_listener lis;
    cute::makeRunner(lis)(s, "The Suite");
}

int main() {
    runSuite();
}
