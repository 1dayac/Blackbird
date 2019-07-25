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
        while(reader.GetNextAlignment(alignment)) {
            std::string bx;
            VERBOSE_POWER(++alignment_count, " alignments processed");
            alignment.GetTag("BX", bx);

            if (!(alignment.CigarData.size() == 1 && alignment.CigarData[0].Type == 'M')) {
                map_of_bad_reads_[bx].push_back(alignment);
                VERBOSE_POWER(map_of_bad_reads_.size(), " alignments stored");
            }
        }
        reader.Close();
        INFO("Blackbird finished");
        return 0;
    }
private:
    std::unordered_map<std::string, std::vector<BamTools::BamAlignment>> map_of_bad_reads_;
};

#endif //BLACKBIRD_PIPELINE_H

