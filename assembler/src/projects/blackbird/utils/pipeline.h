//
// Created by Dima on 7/23/19.
//

#ifndef BLACKBIRD_PIPELINE_H
#define BLACKBIRD_PIPELINE_H
#include "projects/spades/launch.hpp"
#include "utils/logger/log_writers.hpp"
#include "options.h"
#include "io/sam/bam_reader.hpp"

void create_console_logger(std::string log_prop_fn) {
    using namespace logging;
    logger *lg = create_logger(fs::FileExists(log_prop_fn) ? log_prop_fn : "");
    lg->add_writer(std::make_shared<console_writer>());
    //lg->add_writer(std::make_shared<mutex_writer>(std::make_shared<console_writer>()));
    attach_logger(lg);
}

struct RefWindow {

    std::string RefName;    //!< name of reference sequence
    int32_t     WindowStart;  //!< length of reference sequence
    int32_t     WindowEnd;  //!< length of reference sequence

    //! constructor
    RefWindow(const std::string& name,
            const int32_t& windowStart, const int32_t& windowEnd)
            : RefName(name), WindowStart(windowStart)
            , WindowEnd(windowEnd)
    { }

    std::string ToString() {
        std::string out = RefName + " " + std::to_string(WindowStart) + " " + std::to_string(WindowEnd);
        return out;
    }
};


class BlackBirdLauncher {
public:
    int Launch() {
        utils::perf_counter pc;
        std::string log_filename = OptionBase::output_folder + "/blackdird.log";
        create_console_logger(log_filename);
        INFO("Starting Blackbird");
        INFO("Hey, I'm Blackbird");
        BamTools::BamReader reader;
        reader.Open(OptionBase::bam.c_str());
        BamTools::BamAlignment alignment;
        size_t alignment_count = 0;
        size_t alignments_stored = 0;
        while(reader.GetNextAlignment(alignment)) {
            std::string bx;
            VERBOSE_POWER(++alignment_count, " alignments processed");
            alignment.GetTag("BX", bx);
            if (CheckConditions(alignment) && alignment.IsPrimaryAlignment()) {
                //INFO(alignment.Name << " " << alignment.QueryBases);
                map_of_bad_reads_[bx].push_back(alignment.QueryBases);
                VERBOSE_POWER(++alignments_stored, " alignments stored");
            }
        }
        INFO("Total " << alignment_count << " alignments processed");
        INFO("Total " << alignments_stored << " alignments stored");
        reader.Close();


        reader.Open(OptionBase::bam.c_str());
        auto ref_data = reader.GetReferenceData();
        for (auto reference : ref_data) {
            int window_width = 50000;
            int overlap = 10000;
            for (int start_pos = 0; start_pos < reference.RefLength; start_pos += window_width - overlap) {
                RefWindow r(reference.RefName, start_pos, start_pos + window_width);
                INFO(r.ToString());
            }
        }
        INFO("Blackbird finished");
        return 0;
    }
private:
    std::unordered_map<std::string, std::vector<std::string>> map_of_bad_reads_;


    bool CheckConditions(BamTools::BamAlignment &alignment) {
        if (alignment.MapQuality < OptionBase::mapping_quality) {
            return true;
        }
        auto cigar = alignment.CigarData;
        int num_soft_clip = 0;
        for (auto c : cigar) {
            if (c.Type == 'S') {
                num_soft_clip += c.Length;
            }
        }
        if (num_soft_clip/(double)alignment.Length > 0.2/*opt::max_soft_clipping*/) {
            return true;
        }
        return false;
    }


};

#endif //BLACKBIRD_PIPELINE_H

