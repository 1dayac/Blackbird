//
// Created by Dima on 7/23/19.
//

#ifndef BLACKBIRD_PIPELINE_H
#define BLACKBIRD_PIPELINE_H
#include<algorithm>

#include "projects/spades/launch.hpp"
#include "utils/logger/log_writers.hpp"
#include "options.h"
#include "io/sam/bam_reader.hpp"
#include "common/io/reads/osequencestream.hpp"

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
        fs::make_dir(OptionBase::output_folder);
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
            if (IsBadAlignment(alignment) && alignment.IsPrimaryAlignment()) {
                //INFO(alignment.Name << " " << alignment.QueryBases);
                map_of_bad_reads_[bx].push_back(io::SingleRead(alignment.Name, alignment.QueryBases, alignment.Qualities, io::PhredOffset));
                VERBOSE_POWER(++alignments_stored, " alignments stored");
            }
        }
        INFO("Total " << alignment_count << " alignments processed");
        INFO("Total " << alignments_stored << " alignments stored");
        reader.Close();


        reader.Open(OptionBase::bam.c_str());
        if (reader.OpenIndex((OptionBase::bam + ".bai").c_str())) {
            INFO("Index located at " << OptionBase::bam << ".bai");
        } else {
            FATAL_ERROR("Index at " << OptionBase::bam << ".bai" << " can't be located")
        }

        auto ref_data = reader.GetReferenceData();

        BamTools::BamRegion target_region(reader.GetReferenceID("chr13"), 30000000, reader.GetReferenceID("chr13"), 40000000);

        for (auto reference : ref_data) {
            if(target_region.LeftRefID != reader.GetReferenceID(reference.RefName)) {
                continue;
            }
            int window_width = 50000;
            int overlap = 10000;
            for (int start_pos = 0; start_pos < reference.RefLength; start_pos += window_width - overlap) {
                if (start_pos < target_region.LeftPosition || start_pos > target_region.RightPosition) {
                    continue;
                }
                RefWindow r(reference.RefName, start_pos, start_pos + window_width);
                INFO(r.ToString());
                BamTools::BamRegion region(reader.GetReferenceID(reference.RefName), start_pos, reader.GetReferenceID(reference.RefName), start_pos + window_width);
                if (reader.SetRegion(region)) {
                    INFO("Region is set");
                } else {
                    INFO("Region can't be set");
                    continue;
                }
                std::unordered_map<std::string, int> barcodes_count;
                std::set<std::string> barcodes_count_over_threshold;
                const int threshold = 4;
                const int number_of_barcodes_to_assemble = 200;
                while(reader.GetNextAlignment(alignment)) {
                    if (alignment.IsPrimaryAlignment() && IsGoodAlignment(alignment)) {
                        std::string bx = "";
                        alignment.GetTag("BX", bx);
                        if (++barcodes_count[bx] > threshold) {
                            barcodes_count_over_threshold.insert(bx);
                            if (barcodes_count_over_threshold.size() == number_of_barcodes_to_assemble) {
                                break;
                            }
                        }
                    }
                }
                INFO("Taking first " << number_of_barcodes_to_assemble << " barcodes");
                reader.SetRegion(region);

                std::string temp_dir = fs::make_temp_dir(OptionBase::output_folder + "/", "");
                io::OPairedReadStream<std::ofstream, io::FastqWriter> out_stream(temp_dir + "/R1.fastq", temp_dir + "/R2.fastq");
                io::OReadStream<std::ofstream, io::FastqWriter> single_out_stream(temp_dir + "/single.fastq");

                while (reader.GetNextAlignment(alignment)) {
                    if (alignment.Position > start_pos + window_width || alignment.RefID != reader.GetReferenceID(reference.RefName)) {
                        break;
                    }

                    std::string bx = "";
                    alignment.GetTag("BX", bx);
                    if (!barcodes_count_over_threshold.count(bx) || !alignment.IsPrimaryAlignment()) {
                        continue;
                    }


                    if (alignment.MateRefID == -1) {
                        OutputSingleRead(alignment, single_out_stream);
                    } else {
                        OutputPairedRead(alignment, out_stream, reader);
                    }
                }

                for (auto barcode : barcodes_count_over_threshold) {
                    for (auto read : map_of_bad_reads_[barcode]) {
                        single_out_stream << read;
                    }
                }
                return 0;
            }
        }




        INFO("Blackbird finished");
        return 0;
    }
private:
    std::unordered_map<std::string, std::vector<io::SingleRead>> map_of_bad_reads_;


    bool IsBadAlignment(BamTools::BamAlignment &alignment) {
        //very bad alignment
        for (auto ch : alignment.Qualities) {
            if (ch < '5') {
                return false;
            }
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

    bool IsGoodAlignment(BamTools::BamAlignment &alignment) {
        auto cigar = alignment.CigarData;
        if (cigar.size() == 1 && cigar[0].Type == 'M' && alignment.RefID == alignment.MateRefID) {
            return true;
        }
        return false;
    }

    void OutputPairedRead(BamTools::BamAlignment &alignment, io::OPairedReadStream<std::ofstream, io::FastqWriter> &out_stream, BamTools::BamReader &reader) {
        io::SingleRead first;
        io::SingleRead second;
        if (alignment.IsFirstMate()) {
            std::string read_name = alignment.Name;
            first = CreateRead(alignment);
            first = CreateRead(alignment);
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsFirstMate() || !mate_alignment.IsPrimaryAlignment()) {
                reader.GetNextAlignment(mate_alignment);
                if (mate_alignment.Position > alignment.MatePosition) {
                    reader.Jump(alignment.RefID, alignment.Position);
                    while (reader.GetNextAlignment(alignment)) {
                        if (alignment.Name == read_name && alignment.IsFirstMate() && alignment.IsPrimaryAlignment()) {
                            break;
                        }
                    }
                    return;
                }
            }
            second = CreateRead(mate_alignment);
            reader.Jump(alignment.RefID, alignment.Position);
            while (reader.GetNextAlignment(alignment)) {
                if (alignment.Name == read_name && alignment.IsFirstMate() && alignment.IsPrimaryAlignment()) {
                    break;
                }
            }
        } else {
            second = CreateRead(alignment);
            std::string read_name = alignment.Name;
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsSecondMate() || !mate_alignment.IsPrimaryAlignment()) {
                reader.GetNextAlignment(mate_alignment);
                if (mate_alignment.Position > alignment.MatePosition) {
                    reader.Jump(alignment.RefID, alignment.Position);

                    while (reader.GetNextAlignment(alignment)) {
                        if (alignment.Name == read_name && alignment.IsSecondMate() && alignment.IsPrimaryAlignment()) {
                            break;
                        }
                    }
                    return;

                }
            }
            first = CreateRead(mate_alignment);
            reader.Jump(alignment.RefID, alignment.Position);
            while (reader.GetNextAlignment(alignment)) {
                if (alignment.Name == read_name && alignment.IsSecondMate() && alignment.IsPrimaryAlignment()) {
                    break;
                }
            }
        }

        io::PairedRead pair(first, second, 0);
        out_stream << pair;

    }

    io::SingleRead CreateRead(BamTools::BamAlignment &alignment) {
        std::string bases = alignment.IsReverseStrand() ?  ReverseComplement(alignment.QueryBases) : alignment.QueryBases;
        std::string qualities = alignment.Qualities;

        if (alignment.IsReverseStrand()) {
            std::reverse(qualities.begin(), qualities.end());
        }
        return io::SingleRead(alignment.Name, bases, qualities, io::OffsetType::PhredOffset);
    }

    void OutputSingleRead(BamTools::BamAlignment &alignment, io::OReadStream<std::ofstream, io::FastqWriter> &out_stream) {
        io::SingleRead first = io::SingleRead(alignment.Name, alignment.QueryBases, alignment.Qualities, io::OffsetType::PhredOffset);
        out_stream << first;
    }

};

#endif //BLACKBIRD_PIPELINE_H

