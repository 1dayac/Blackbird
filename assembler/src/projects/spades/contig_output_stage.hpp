//***************************************************************************
//* Copyright (c) 2015-2017 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#pragma once

#include "pipeline/stage.hpp"

namespace debruijn_graph {


class ContigOutput : public spades::AssemblyStage {
public:
    enum class Kind {
        BinaryContigs,
        EdgeSequences,
        GFAGraph,
        FASTGGraph,
        FinalContigs,
        PlasmidContigs,
        Scaffolds
    };
    typedef std::map<Kind, std::string> OutputList;

    ContigOutput(OutputList list)
            : AssemblyStage("Contig Output", "contig_output"),
              outputs_(std::move(list)) {}

    void load(GraphPack &, const std::string &, const char *) { }
    void save(const GraphPack &, const std::string &, const char *) const { }
    void run(GraphPack &gp, const char *);

private:
    OutputList outputs_;
};

}
