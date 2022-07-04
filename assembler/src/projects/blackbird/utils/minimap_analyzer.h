//as
// Created by dmm2017 on 5/13/22.
//

#ifndef SRC_MINIMAP_ANALYZER_H
#define SRC_MINIMAP_ANALYZER_H

#include "common/assembly_graph/paths/mapping_path.hpp"


using omnigraph::MappingRange;
struct AlignBlock {
    MappingRange range_;
    std::vector<std::pair<char, int>> cigar_string_;
    AlignBlock(int ref_start, int ref_end, int query_start, int query_end, const std::vector<std::pair<char, int>> &cigar_string)
    : range_(ref_start, ref_end, query_start, query_end), cigar_string_(cigar_string) {

    }
};


typedef std::vector<AlignBlock> AlignVector;

struct ScoredSet{
    double score_;
    std::vector<size_t> indexes_;
    int uncovered_;
    ScoredSet(int contig_length)
    :score_(0), uncovered_(contig_length) {}
};


static void AnalyzeBlocks(AlignVector &aligned_blocks, AlignVector &top_blocks, int contig_size) {
    std::sort(aligned_blocks.begin(), aligned_blocks.end(), [](AlignBlock & a, AlignBlock & b) -> bool
    {
        return a.range_.initial_range.size() > b.range_.initial_range.size();
    });
    if (aligned_blocks.size() == 0)
        return;
    if (aligned_blocks.size() == 1 || aligned_blocks.front().range_.initial_range.size() > contig_size * 0.99) {
        top_blocks.push_back(aligned_blocks.front());
        return;
    }

    std::map<std::string, double> penalties;
    penalties["extensive"] = std::max(50, std::min(250, int(round(contig_size * 0.05)))) - 1;
    penalties["local"] = std::max(2, std::min(25, int(round(contig_size * 0.01)))) - 1;
    penalties["overlap_multiplier"] = 0.5;

    std::vector<ScoredSet> all_sets;
    all_sets.push_back(ScoredSet(contig_size));


    int max_score = 0;
    int current_solid_idx = -1;
    int next_solid_idx = -1;
    AlignVector solid;

    for (int i = 0; i < aligned_blocks.size(); ++i) {
        int local_max_score = 0;
    }
}

#endif //SRC_MINIMAP_ANALYZER_H
