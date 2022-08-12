//
// Created by Dmitry Meleshko on 7/23/19.
//
#pragma once

#include<algorithm>
#include <list>
#include <cassert>
#include <utility>
#include <type_traits>
#include <unordered_set>
#include "utils/logger/log_writers.hpp"
#include "options.h"
#include "io/sam/bam_reader.hpp"
#include "common/io/reads/osequencestream.hpp"
#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/phmap_fwd_decl.h"
#include "common/assembly_graph/paths/mapping_path.hpp"

#include <minimap2/minimap.h>
#include <bamtools/api/BamWriter.h>

#include "io/reads/fasta_fastq_gz_parser.hpp"
#include "common/utils/parallel/openmp_wrapper.h"
#include "common/utils/memory_limit.hpp"
#include "svs.h"
#include "minimap_analyzer.h"

static void create_console_logger() {
    using namespace logging;

    logger *lg = create_logger("");
    lg->add_writer(std::make_shared<console_writer>());
    attach_logger(lg);
}




typedef phmap::parallel_flat_hash_map<std::string, std::pair<Sequence, std::string>,
        phmap::container_internal::hash_default_hash<std::string>,
        phmap::container_internal::hash_default_eq<std::string>,
        phmap::container_internal::Allocator<
                phmap::container_internal::Pair<const std::string, std::pair<Sequence, std::string>>>,
        4, phmap::NullMutex> ReadMap;

typedef phmap::parallel_flat_hash_map<std::string, Sequence,
        phmap::container_internal::hash_default_hash<std::string>,
        phmap::container_internal::hash_default_eq<std::string>,
        phmap::container_internal::Allocator<
                phmap::container_internal::Pair<const std::string, std::pair<Sequence, std::string>>>,
        4, phmap::NullMutex> LongReadMap;

struct RefWindow {

    BamTools::RefData RefName;    //!< name of reference sequence
    int32_t           WindowStart;  //!< length of reference sequence
    int32_t           WindowEnd;  //!< length of reference sequence

    //! constructor
    RefWindow(const BamTools::RefData& reference,
              const int32_t& windowStart, const int32_t& windowEnd)
            : RefName(reference), WindowStart(windowStart)
            , WindowEnd(windowEnd)
    { }

    std::string ToString() const {
        std::string out = RefName.RefName + " " + std::to_string(WindowStart) + " " + std::to_string(WindowEnd);
        return out;
    }
};

class VCFWriter {
    std::ofstream file_;
public:
    VCFWriter() = default;

    VCFWriter(const std::string &filename)
    {
        file_ = std::ofstream(filename, std::ofstream::out);

    }

    void WriteHeader(const std::unordered_map<std::string, std::string> &reference_map) {
        file_ << "##fileformat=VCFv4.2" << std::endl;
        file_ << "##source=Blackbird-v0.1" << std::endl;
        file_ << "##command=" << OptionBase::cmd_line << std::endl;
        file_ << "##reference=" << OptionBase::reference << std::endl;
        for (const auto &p : reference_map) {
            file_ << "##contig=<ID=" << p.first << ",length=" << p.second.size() << ">" << std::endl;
        }
        file_ << "##INFO=<ID=SEQ,Number=1,Type=String,Description=\"SV sequence\">" << std::endl;
        file_ << "##INFO=<ID=SVTYPE,Number=1,Type=String,Description=\"Type of variant, either DEL, INV, or INS\">" << std::endl;
        file_ << "##INFO=<ID=SVLEN,Number=1,Type=Integer,Description=\"Difference in length between REF and ALT alleles\">" << std::endl;
        file_ << "##INFO=<ID=END,Number=1,Type=Integer,Description=\"End position of the variant described in this record\">" << std::endl;
        file_ << "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">" << std::endl;
        file_ << "#CHROM" << "\t" << "POS" << "\t" << "ID" << "\t" << "REF" << "\t" << "ALT" << "\t" << "QUAL" << "\t" << "FILTER" << "\t" << "INFO" << "\t" << "FORMAT" << "\t" << "SAMPLE1"<< std::endl;

    }

    void Write(const Deletion &sv) {
        file_ << sv.ToString() << std::endl;
    }

    void Write(const Insertion &sv) {
        file_ << sv.ToString() << std::endl;
    }

    void Write(const Inversion &sv) {
        file_ << sv.ToString() << std::endl;
    }
};

class BlackBirdLauncher {

    template<typename T>
    void ProcessInversion(T *r, const std::string &query, const std::string &ref_name, int start_pos) {
        std::string inversion_seq = "";
        int query_start = r->qs;
        int reference_start = r->rs;
        if (!r->rev) {
            for (int i = 0; i < r->p->n_cigar; ++i) {
                //printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                    inversion_seq = query.substr(query_start, r->p->cigar[i]>>4);
                    query_start += r->p->cigar[i]>>4;
                    reference_start += r->p->cigar[i]>>4;
                }
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                    inversion_seq += query.substr(query_start, r->p->cigar[i]>>4);
                    query_start += r->p->cigar[i]>>4;
                }
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                    reference_start += r->p->cigar[i]>>4;
                }
            }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
        } else {
            for (int i = 0; i < r->p->n_cigar; ++i) {
                //printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                    inversion_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4) + inversion_seq;
                    query_start += r->p->cigar[i]>>4;
                    reference_start += r->p->cigar[i]>>4;
                }
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                    inversion_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4) + inversion_seq;
                    query_start += r->p->cigar[i]>>4;
                }
                if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                    reference_start += r->p->cigar[i]>>4;
                }
            }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
        }
        Inversion inv(ref_name, start_pos + r->rs, start_pos + query_start, inversion_seq, inversion_seq.back());
        WriteCritical(vector_of_inv_, inv);
    }


    void test_minimap(const std::string &reference, const std::string &contig) {
        io::FastaFastqGzParser reference_reader(reference);
        io::SingleRead reference_contig;
        reference_reader >> reference_contig;
        RunAndProcessMinimap(contig, reference_contig.GetSequenceString(), "1", 0);
        std::sort(vector_of_ins_.begin(), vector_of_ins_.end());
        std::sort(vector_of_del_.begin(), vector_of_del_.end());
        Print(vector_of_ins_, vector_of_del_, writer_);
    }


public:
    BlackBirdLauncher () = default;

    int Launch() {
        srand(113018);
        utils::perf_counter pc;
        std::string log_filename = "blackbird.log";

        fs::make_dir(OptionBase::output_folder);



        writer_ = VCFWriter(OptionBase::output_folder + "/out_50.vcf");
        writer_small_ = VCFWriter(OptionBase::output_folder + "/out.vcf");
        writer_inversion_ = VCFWriter(OptionBase::output_folder + "/out_inversions.vcf");

        create_console_logger();
        INFO("Starting Blackbird");


//        test_minimap("/home/dmm2017/Desktop/blackbird_debug/chr5_360000_410000/subref.fasta", "/home/dmm2017/Desktop/blackbird_debug/chr5_360000_410000/assembly/contigs_rc.fasta");
//        return 0;


        int max_treads = omp_get_max_threads();


        if (OptionBase::threads > max_treads) {
            WARN("Only " << max_treads << " thread(s) available.");
            OptionBase::threads = max_treads;
        }



        INFO("Number of threads being used - " << OptionBase::threads);

        INFO("Memory limit  " << (1.0 * (double) utils::get_memory_limit() / 1024 / 1024 / 1024) << " Gb");
        INFO("Free memory - " << utils::get_free_memory());
        if (OptionBase::use_long_reads)
            INFO("Running in hybrid (linked read + long read) mode");

        INFO("Uploading reference genome");
        utils::limit_memory(utils::get_memory_limit() * 2);
        INFO("Memory limit  " << (1.0 * (double) utils::get_memory_limit() / 1024 / 1024 / 1024) << " Gb");
        io::FastaFastqGzParser reference_reader(OptionBase::reference);
        io::SingleRead chrom;
        while (!reference_reader.eof()) {
            reference_reader >> chrom;
            std::string name = chrom.name().find(' ') == std::string::npos ? chrom.name() : chrom.name().substr(0, chrom.name().find(' '));
            INFO("Adding " << name << " to the map");
            reference_map_[name] = chrom.GetSequenceString();
        }

        std::string bam_filename = OptionBase::bam.find_last_of('/') == std::string::npos ? OptionBase::bam : OptionBase::bam.substr(OptionBase::bam.find_last_of('/') + 1);
        std::string new_bam_name = OptionBase::output_folder + "/" + bam_filename.substr(0, bam_filename.length() - 4).c_str() + "filtered.bam";
        if (OptionBase::dont_collect_reads) {
            new_bam_name = OptionBase::bam;
        }

        BamTools::BamReader preliminary_reader;
        preliminary_reader.Open(OptionBase::bam.c_str());


        BamTools::BamAlignment alignment;
        ReadMap map_of_bad_first_reads_;
        ReadMap map_of_bad_second_reads_;



        if (!OptionBase::dont_collect_reads) {
            INFO("Start filtering reads with bad AM tag...");
/*
            BamTools::BamReader temp_reader;
            temp_reader.Open(OptionBase::bam);
            auto ref_data = temp_reader.GetReferenceData();
            std::vector<RefWindow> reference_windows;
            CreateReferenceWindows(reference_windows, ref_data, 0);
            temp_reader.Close();

            std::vector<BamTools::BamReader> filtering_readers(OptionBase::threads);

            for (auto &r : filtering_readers) {
                r.Open(OptionBase::bam.c_str());
                if (r.OpenIndex((OptionBase::bam + ".bai").c_str())) {
                    INFO("Index located at " << OptionBase::bam << ".bai");
                } else {
                    FATAL_ERROR("Index at " << OptionBase::bam << ".bai" << " can't be located")
                }
            }


            #pragma omp parallel for schedule(dynamic, 1) num_threads(OptionBase::threads)
            for (int i = 0; i < reference_windows.size(); ++i) {
                FilterInWindow(reference_windows[i], filtering_readers[omp_get_thread_num()]);
                INFO(i << " " << omp_get_thread_num());
            }

*/

            BamTools::BamWriter writer;
            writer.Open(new_bam_name, preliminary_reader.GetConstSamHeader(), preliminary_reader.GetReferenceData());
            long long total = 0;
            while (preliminary_reader.GetNextAlignmentCore(alignment)) {
                if (!alignment.IsPrimaryAlignment())
                    continue;
                if (alignment.IsDuplicate())
                    continue;

                if (alignment.MapQuality == 60) {
                    writer.SaveAlignment(alignment);
                    continue;
                }

                std::string am_tag;
                alignment.GetTagCore("AM", am_tag);

                if (am_tag == "0") {
                    std::string bx;

                    alignment.GetTagCore("BX", bx);
                    if (bx == "") {
                        continue;
                    }
                    alignment.BuildCharData();
                    if (alignment.IsFirstMate()) {
//                        INFO(alignment.Name);
//                        INFO(alignment.QueryBases);
                        std::string query = alignment.QueryBases.find_last_of("N") == std::string::npos ? alignment.QueryBases : alignment.QueryBases.substr(alignment.QueryBases.find_last_of("N") + 1);
                        if (!query.empty() && !IsDegenerate(query)) {
                            map_of_bad_first_reads_[alignment.Name] = {Sequence(query), bx};
                            VERBOSE_POWER(++total, " reads filtered");
                        }
                    }
                    else {
 //                       INFO(alignment.Name);
 //                       INFO(alignment.QueryBases);
                        std::string query = alignment.QueryBases.find_last_of("N") == std::string::npos ? alignment.QueryBases : alignment.QueryBases.substr(alignment.QueryBases.find_last_of("N") + 1);
                        if (!query.empty() && !IsDegenerate(query)) {
                            map_of_bad_second_reads_[alignment.Name] = {Sequence(query), bx};
                            VERBOSE_POWER(++total, " reads filtered");
                        }
                    }
                    continue;
                }
                writer.SaveAlignment(alignment);
            }
            writer.Close();
            INFO(total << " reads filtered total.");
        }

        if (OptionBase::use_long_reads) {
            io::FastaFastqGzParser long_read_parser(OptionBase::long_read_fastq);
            io::SingleRead long_read;
            while (!long_read_parser.eof()) {
                long_read_parser >> long_read;
                map_of_long_reads_[long_read.name()] = {long_read.sequence()};
            }
            INFO(map_of_long_reads_.size() << " long reads added to the memory");
        }


        BamTools::BamReader reader;
        BamTools::BamReader mate_reader;

        if (!OptionBase::dont_collect_reads) {
            reader.Open(new_bam_name.c_str());
            INFO(new_bam_name);
            mate_reader.Open(new_bam_name.c_str());
            INFO("Creating index file...");
            reader.CreateIndex(BamTools::BamIndex::STANDARD);
        }
        else {
            reader.Open(OptionBase::bam.c_str());
            mate_reader.Open(OptionBase::bam.c_str());
            new_bam_name = OptionBase::bam;
        }


        auto ref_data = reader.GetReferenceData();

        for (auto& reference : ref_data) {
            refid_to_ref_name_[reader.GetReferenceID(reference.RefName)] = reference.RefName;
        }

        if (!OptionBase::dont_collect_reads) {


            size_t alignment_count = 0;
            size_t alignments_stored = 0;
            size_t bad_read_pairs_stored = 0;

            int current_refid = -1;
            INFO("Filter poorly aligned reads");
            while(reader.GetNextAlignmentCore(alignment)) {
                VERBOSE_POWER(++alignment_count, " alignments processed");
                if (alignment.RefID != current_refid) {
                    current_refid = alignment.RefID;
                    DEBUG("Processing chromosome " << refid_to_ref_name_[current_refid]);
                }
                if (!alignment.IsPrimaryAlignment())
                    continue;
                alignment.BuildCharData();
                if (IsBadAlignment(alignment, refid_to_ref_name_)) {
                    if (alignment.QueryBases.find("N") == std::string::npos && !IsDegenerate(alignment.QueryBases)) {

                        std::string bx;
                        alignment.GetTagCore("BX", bx);
                        if (bx == "") {
                            continue;
                        }
                        std::string query = alignment.QueryBases;
                        if (query.empty())
                            continue;
                        if (alignment.IsFirstMate())
                            map_of_bad_first_reads_[alignment.Name] = {Sequence(query), bx};
                        else
                            map_of_bad_second_reads_[alignment.Name] = {Sequence(query), bx};
                        VERBOSE_POWER(++alignments_stored, " alignments stored");
                    }
                }
            }
            for (auto read : map_of_bad_first_reads_) {
                if (map_of_bad_second_reads_.count(read.first)) {
                    map_of_bad_read_pairs_[read.second.second].push_back({read.second.first, map_of_bad_second_reads_[read.first].first});
                    bad_read_pairs_stored++;
                } else {
                    map_of_bad_reads_[read.second.second].push_back(read.second.first);
                }
            }

            for (auto read : map_of_bad_second_reads_) {
                if (!map_of_bad_first_reads_.count(read.first)) {
                    map_of_bad_reads_[read.second.second].push_back(read.second.first);
                }
            }

            map_of_bad_first_reads_.clear();
            map_of_bad_second_reads_.clear();
            INFO("Total " << alignment_count << " alignments processed");
            INFO("Total " << alignments_stored << " alignments stored");
            INFO("Total " << bad_read_pairs_stored << " bad read pairs stored");
            reader.Close();
        } else {
            INFO("Read collection is disabled. Use for debug purposes only!");
        }

        //BamTools::BamRegion target_region(reader.GetReferenceID("chr1"), 0, reader.GetReferenceID("chr1"), 300000000);
        INFO("Create reference windows");


        std::vector<RefWindow> reference_windows;
        CreateReferenceWindows(reference_windows, ref_data, 10000);

        std::vector<BamTools::BamReader> readers(OptionBase::threads);
        std::vector<BamTools::BamReader> mate_readers(OptionBase::threads);
        std::vector<BamTools::BamReader> long_read_readers(OptionBase::threads);

        for (auto &r : readers) {
            r.Open(new_bam_name.c_str());
            if (r.OpenIndex((new_bam_name + ".bai").c_str())) {
                INFO("Index located at " << new_bam_name << ".bai");
            } else {
                FATAL_ERROR("Index at " << new_bam_name << ".bai" << " can't be located")
            }

        }

        for (auto &r : mate_readers) {
            r.Open(new_bam_name.c_str());
            r.OpenIndex((new_bam_name + ".bai").c_str());
        }

        if (OptionBase::use_long_reads) {
            for (auto &r : long_read_readers) {
                r.Open(OptionBase::long_read_bam.c_str());
                r.OpenIndex((OptionBase::long_read_bam + ".bai").c_str());
            }
        }


#pragma omp parallel for schedule(dynamic, 1) num_threads(OptionBase::threads)
        for (int i = 0; i < reference_windows.size(); ++i) {
            ProcessWindow(reference_windows[i], readers[omp_get_thread_num()], mate_readers[omp_get_thread_num()], long_read_readers[omp_get_thread_num()]);
            INFO(i << " " << omp_get_thread_num());
        }

        std::sort(vector_of_ins_.begin(), vector_of_ins_.end());
        std::sort(vector_of_del_.begin(), vector_of_del_.end());

        std::sort(vector_of_small_ins_.begin(), vector_of_small_ins_.end());
        std::sort(vector_of_small_del_.begin(), vector_of_small_del_.end());

        std::sort(vector_of_inv_.begin(), vector_of_inv_.end());
        writer_.WriteHeader(reference_map_);
        writer_small_.WriteHeader(reference_map_);
        Print(vector_of_ins_, vector_of_del_, writer_);
        Print(vector_of_small_ins_, vector_of_small_del_, writer_small_);
        PrintInversions(vector_of_inv_, writer_inversion_);
        //test_minimap();
        for (auto &r : readers) {
            r.Close();
        }
        for (auto &r : mate_readers) {
            r.Close();
        }

        INFO("Blackbird finished");
        return 0;
    }

private:

    phmap::parallel_flat_hash_map<std::string, std::vector<Sequence>,
            phmap::container_internal::hash_default_hash<std::string>,
            phmap::container_internal::hash_default_eq<std::string>,
            phmap::container_internal::Allocator<
                    phmap::container_internal::Pair<const std::string, std::vector<Sequence>>>,
            4, phmap::NullMutex> map_of_bad_reads_;

    phmap::parallel_flat_hash_map<std::string, std::vector<std::pair<Sequence, Sequence>>,
            phmap::container_internal::hash_default_hash<std::string>,
            phmap::container_internal::hash_default_eq<std::string>,
            phmap::container_internal::Allocator<
                    phmap::container_internal::Pair<const std::string, std::vector<std::pair<Sequence, Sequence>>>>,
            4, phmap::NullMutex> map_of_bad_read_pairs_;

    LongReadMap map_of_long_reads_;

    VCFWriter writer_;
    VCFWriter writer_small_;
    VCFWriter writer_inversion_;
    std::vector<Insertion> vector_of_small_ins_;
    std::vector<Deletion> vector_of_small_del_;
    std::vector<Insertion> vector_of_ins_;
    std::vector<Deletion> vector_of_del_;
    std::vector<Inversion> vector_of_inv_;

    std::unordered_map<std::string, std::string> reference_map_;
    std::unordered_map<int, std::string> refid_to_ref_name_;
    static int uniq_number;
    static int jumps;

    void PrintInversions(const std::vector<Inversion> &vector_of_inv, VCFWriter &writer) {
        for (auto inv : vector_of_inv) {
            writer.Write(inv);
        }
    }

    template<typename T>
    bool AlmostEqual(const T &sv1, const T &sv2) {
        return sv1.Size() == sv2.Size() && sv1.chrom_ == sv2.chrom_ && abs(sv1.ref_position_ - sv2.ref_position_) < 10;
    }

    void Print(const std::vector<Insertion> &vector_of_ins, const std::vector<Deletion> &vector_of_del, VCFWriter &writer) {
        int i = 0;
        int j = 0;
        while (i != vector_of_ins.size() || j != vector_of_del.size()) {
            if (i == vector_of_ins.size()) {
                writer.Write(vector_of_del[j]);
                j++;
                while(j != vector_of_del.size() && AlmostEqual(vector_of_del[j], vector_of_del[j - 1])) {
                    ++j;
                }
                continue;
            }
            if (j == vector_of_del.size()) {
                writer.Write(vector_of_ins[i]);
                ++i;
                while(i != vector_of_ins.size() && AlmostEqual(vector_of_ins[i], vector_of_ins[i - 1])) {
                    ++i;
                }
                continue;
            }

            if (vector_of_del[j] < vector_of_ins[i]) {
                writer.Write(vector_of_del[j]);
                j++;
                while(j != vector_of_del.size() && AlmostEqual(vector_of_del[j], vector_of_del[j - 1])) {
                    ++j;
                }
            } else {
                writer.Write(vector_of_ins[i]);
                i++;
                while(i != vector_of_ins.size() && AlmostEqual(vector_of_ins[i], vector_of_ins[i - 1])) {
                    ++i;
                }
            }
        }
    }



    void CreateReferenceWindows(std::vector<RefWindow> &reference_windows, BamTools::RefVector& ref_data, int overlap = 10000) {
        int number_of_windows = 0;
        int window_width = 50000;
        if (OptionBase::region_file == "") {
            for (auto reference : ref_data) {
                //if(target_region.LeftRefID != reader.GetReferenceID(reference.RefName)) {
                //    continue;
                //}
                if (!IsGoodRef(reference.RefName) || !reference_map_.count(reference.RefName)) {
                    continue;
                }
                for (int start_pos = 0; start_pos < reference.RefLength; start_pos += window_width - overlap) {
                    //if (start_pos < target_region.LeftPosition || start_pos > target_region.RightPosition || reference.RefName != "chr1") {
                    //    continue;
                    //}
                    RefWindow r(reference.RefName, start_pos, start_pos + window_width);
                    reference_windows.push_back(r);
                    ++number_of_windows;
                }
            }
        } else {
            std::ifstream region_file(OptionBase::region_file);
            while(!region_file.eof()) {
                std::string chrom = "";
                int start = 0;
                int end = 0;
                region_file >> chrom >> start >> end;
                end = std::min<unsigned long>(end, reference_map_[chrom].size());
                INFO("Chromosome " << chrom << ":" << start << "-" << end);
                if (!IsGoodRef(chrom) || !reference_map_.count(chrom)) {
                    continue;
                }
                for (int start_pos = start; start_pos < end; start_pos += window_width - overlap) {
                    RefWindow r(chrom, start_pos, start_pos + window_width);
                    reference_windows.push_back(r);
                    ++number_of_windows;
                }
            }
        }
        INFO(number_of_windows << " totally created.");
    }

    void FilterInWindow(const RefWindow &window, BamTools::BamReader &reader) {


    }

    void ProcessWindow(const RefWindow &window,  BamTools::BamReader &reader, BamTools::BamReader &mate_reader, BamTools::BamReader &long_read_reader) {
        INFO("Processing " << window.RefName.RefName << " " << window.WindowStart << "-" << window.WindowEnd << " (thread " << omp_get_thread_num() << ")");
        BamTools::BamRegion region(reader.GetReferenceID(window.RefName.RefName), window.WindowStart, reader.GetReferenceID(window.RefName.RefName), window.WindowEnd);
        BamTools::BamRegion extended_region(reader.GetReferenceID(window.RefName.RefName), std::max(0, (int)window.WindowStart - 1000), reader.GetReferenceID(window.RefName.RefName), window.WindowEnd + 1000);
        BamTools::BamRegion short_extended_region(reader.GetReferenceID(window.RefName.RefName), std::max(0, (int)window.WindowStart - 200), reader.GetReferenceID(window.RefName.RefName), window.WindowEnd + 200);

        BamTools::BamAlignment alignment;
        if (!reader.SetRegion(region)) {
            return;
        }
        std::unordered_map<std::string, int> barcodes_count;
        std::set<std::string> barcodes_count_over_threshold_prelim;
        std::unordered_set<std::string> barcodes_count_over_threshold;

        auto const& const_refid_to_ref_name = refid_to_ref_name_;

        std::map<std::string, std::pair<int, bool>> long_read_names;
        if (OptionBase::use_long_reads) {
            long_read_reader.SetRegion(extended_region);
            while(long_read_reader.GetNextAlignment(alignment)) {
                if (alignment.IsPrimaryAlignment())
                    long_read_names[alignment.Name] = {alignment.Position, alignment.IsReverseStrand()};
            }
        }

        const int threshold = 4;
        const int number_of_barcodes_to_assemble = 2500;
        std::map<std::string, std::vector<BamTools::BamAlignment>> barcode_to_alignment_map;
        while(reader.GetNextAlignmentCore(alignment)) {
            if(!alignment.IsPrimaryAlignment())
                continue;
            std::string bx = "";
            alignment.GetTagCore("BX", bx);
            if (bx == "") {
                continue;
            }
            barcode_to_alignment_map[bx].push_back(alignment);
            if (++barcodes_count[bx] > threshold) {
                barcodes_count_over_threshold_prelim.insert(bx);
            }
        }
        std::vector<std::string> barcodes_count_over_threshold_v(barcodes_count_over_threshold_prelim.begin(),
                                                                 barcodes_count_over_threshold_prelim.end());
        std::random_shuffle(barcodes_count_over_threshold_v.begin(), barcodes_count_over_threshold_v.end());
        DEBUG("Taking first " << number_of_barcodes_to_assemble << " barcodes");
        for (int i = 0; i < number_of_barcodes_to_assemble && i < barcodes_count_over_threshold_v.size(); ++i) {
            barcodes_count_over_threshold.insert(barcodes_count_over_threshold_v[i]);
        }
        reader.SetRegion(region);
        std::string temp_dir = OptionBase::output_folder + "/" +
                               const_cast<std::unordered_map<int, std::string>&>(refid_to_ref_name_).at(region.RightRefID) + "_" + std::to_string(region.LeftPosition) + "_" + std::to_string(region.RightPosition);
        fs::make_dir(temp_dir);
        io::OPairedReadStream<std::ofstream, io::FastqWriter> out_stream(temp_dir + "/R1.fastq", temp_dir + "/R2.fastq");
        io::OReadStream<std::ofstream, io::FastqWriter> single_out_stream(temp_dir + "/single.fastq");

        if (OptionBase::use_long_reads) {
            io::OReadStream<std::ofstream, io::FastqWriter> long_read_stream(temp_dir + "/long_reads.fastq");
            std::vector<std::string> long_read_names_vec;
            for (auto p : long_read_names)
                long_read_names_vec.push_back(p.first);
            std::random_shuffle(long_read_names_vec.begin(), long_read_names_vec.end());
            for (int i = 0; i < std::min(200, (int)long_read_names_vec.size()); ++i) {
                auto name = long_read_names_vec[i];
                size_t start_pos = long_read_names[name].first;
                size_t read_length = map_of_long_reads_[name].size();
                int cut_start = 0;
                int cut_end = 0;
                if (start_pos < region.LeftPosition) {
                    cut_start = region.LeftPosition - start_pos;
                }
                if (start_pos + read_length > region.RightPosition) {
                    cut_end = start_pos + read_length - region.RightPosition;
                }

                if (cut_start + cut_end < read_length ) {
                    if (long_read_names[name].second) {
                        auto read_seq = Sequence(map_of_long_reads_[name], true);
                        auto read = read_seq.Subseq(cut_start, map_of_long_reads_[name].size() - cut_end).str();
                        io::SingleRead l(name, read, std::string(read.length(), 'K'));
                        long_read_stream <<  l;
                    } else {
                        auto read = map_of_long_reads_[name].Subseq(cut_start, map_of_long_reads_[name].size() - cut_end).str();
                        io::SingleRead l(name, read, std::string(read.length(), 'K'));
                        long_read_stream <<  l;
                    }
                }
            }
        }

        std::unordered_map<std::string, std::vector<BamTools::BamAlignment>> filtered_reads;

        for (auto p : barcode_to_alignment_map) {
            if (!barcodes_count_over_threshold.count(p.first))
                continue;
            for (auto &alignment : p.second) {
                alignment.BuildCharData();
                filtered_reads[alignment.Name].push_back(alignment);
            }
        }
        barcode_to_alignment_map.clear();
/*

        while (reader.GetNextAlignment(alignment)) {
            if (alignment.Position > region.RightPosition || alignment.RefID != reader.GetReferenceID(window.RefName.RefName)) {
                break;
            }
            std::string bx = "";
            alignment.GetTag("BX", bx);
            if (!barcodes_count_over_threshold.count(bx) || !alignment.IsPrimaryAlignment()) {
                continue;
            }
            filtered_reads[alignment.Name].push_back(alignment);
        }
*/

        int count_add = 0;
        reader.SetRegion(short_extended_region);
        bool did_a_jump = false;
        while (reader.GetNextAlignmentCore(alignment)) {
            if (alignment.Position > short_extended_region.RightPosition || alignment.RefID != reader.GetReferenceID(window.RefName.RefName)) {
                break;
            }
            if (alignment.Position < region.RightPosition && alignment.Position > region.LeftPosition) {
                if (!did_a_jump) {
                    did_a_jump = true;
                    reader.Jump(reader.GetReferenceID(window.RefName.RefName), region.RightPosition);
                }
                continue;
            }
            std::string bx = "";
            alignment.GetTagCore("BX", bx);

            if (!barcodes_count_over_threshold.count(bx) || !alignment.IsPrimaryAlignment()) {
                continue;
            }
            alignment.BuildCharData();
            if (filtered_reads[alignment.Name].size() != 1)
                continue;

            if (filtered_reads[alignment.Name].front().IsFirstMate() !=  alignment.IsFirstMate())
            {
                filtered_reads[alignment.Name].push_back(alignment);
                count_add++;
            }
        }

        std::string barcode_file = temp_dir + "/barcodes.txt";
        std::ofstream barcode_output(barcode_file.c_str(), std::ofstream::out);
        for (auto const& barcode : barcodes_count_over_threshold) {
            barcode_output << barcode << std::endl;
        }

        bool have_singles = false;
        for (auto p : filtered_reads) {
            if (p.second.size() == 1) {
                //if (alignment.MateRefID == -1) {
                have_singles = true;
                OutputSingleRead(p.second[0], single_out_stream);
                //}
                //else {
                //   if (p.second[0].RefID == p.second[0].MateRefID && abs((int)p.second[0].Position - (int)p.second[0].MatePosition) < 500) {
                //       OutputSingleRead(p.second[0], single_out_stream);
                //       continue;
                //   }
                //  if (!OutputPairedRead(p.second[0], out_stream, mate_reader))
                //      OutputSingleRead(p.second[0], single_out_stream);
                // }
            }
            if (p.second.size() == 2) {
                io::SingleRead first = CreateRead(p.second[0]);
                io::SingleRead second = CreateRead(p.second[1]);
                if (p.second[0].IsSecondMate()) {
                    std::swap(first, second);
                }
                io::PairedRead pair(first, second, 0);
                out_stream << pair;
            }
        }


        auto const &const_map_of_bad_read_pairs = map_of_bad_read_pairs_;
        for (auto barcode : barcodes_count_over_threshold) {
            if (const_map_of_bad_read_pairs.count(barcode)) {
                for (auto const &read : const_cast<std::vector<std::pair<Sequence, Sequence>>&>(const_map_of_bad_read_pairs.at(barcode))) {
                    out_stream << CreatePairedReadFromSeq(read.first, read.second);
                }
            }
        }


        auto const &const_map_of_bad_reads = map_of_bad_reads_;
        for (auto barcode : barcodes_count_over_threshold) {
            if (const_map_of_bad_reads.count(barcode)) {
                for (auto const &read : const_cast<std::vector<Sequence>&>(const_map_of_bad_reads.at(barcode))) {
                    have_singles = true;
                    single_out_stream << CreateReadFromSeq(read);
                }
            }
        }




        std::string spades_command = OptionBase::path_to_spades + " --only-assembler -k 55 -t 1 --pe1-1 " + temp_dir + "/R1.fastq --pe1-2 " + temp_dir + "/R2.fastq --pe1-s " + temp_dir + "/single.fastq -o  " + temp_dir + "/assembly >/dev/null";
        if (!have_singles)
            spades_command = OptionBase::path_to_spades + " --only-assembler  -k 55 -t 1 --pe1-1 " + temp_dir + "/R1.fastq --pe1-2 " + temp_dir + "/R2.fastq -o  " + temp_dir + "/assembly >/dev/null";
        if (OptionBase::use_long_reads) {
            spades_command += " --pacbio " + temp_dir + "/long_reads.fastq";
        }
        std::system(spades_command.c_str());
        auto const& const_reference_map = reference_map_;
        std::string subreference = const_reference_map.at(const_refid_to_ref_name.at(region.RightRefID)).substr(region.LeftPosition, region.RightPosition - region.LeftPosition);
        int hits = RunAndProcessMinimap(temp_dir + "/assembly/contigs.fasta", subreference, window.RefName.RefName, region.LeftPosition);
        if (!OptionBase::keep_assembly_folders)
            fs::remove_dir(temp_dir.c_str());
    }

    template<class T>
    void WriteCritical(std::vector<T> &v, const T& t) {
        INFO(t.ToString());
#pragma omp critical
        {
            v.push_back(t);
        }
    }

    io::SingleRead CreateReadFromSeq(const Sequence &seq) {
        uniq_number++;
        return io::SingleRead(std::to_string(uniq_number), seq.str(), std::string(seq.size(), 'J'));
    }

    io::PairedRead CreatePairedReadFromSeq(const Sequence &seq, const Sequence &seq2) {
        int current_id = uniq_number++;
        return io::PairedRead(io::SingleRead(std::to_string(current_id), seq.str(), std::string(seq.size(), 'J')),
                              io::SingleRead(std::to_string(current_id), seq2.str(), std::string(seq2.size(), 'J')), 0);
    }


    bool NoID(mm_reg1_t *r, int index) {
        if (index + 1 == r->p->n_cigar)
            return true;
        if (index == 0)
            return true;

        size_t size = r->p->cigar[index]>>4;
        size_t size_next = r->p->cigar[index + 1]>>4;
        size_t size_prev = r->p->cigar[index - 1]>>4;

        if (index - 1 == 0 && size_prev < 300 && size >= 50)
            return false;

        if (index + 2 == r->p->n_cigar && size_next < 300 && size >= 50)
            return false;

        if ("MIDNSH"[r->p->cigar[index]&0xf] + "MIDNSH"[r->p->cigar[index + 1]&0xf] == 'D' + 'I') {
            return false;
        }
        if ("MIDNSH"[r->p->cigar[index]&0xf] + "MIDNSH"[r->p->cigar[index - 1]&0xf] == 'D' + 'I') {
            return false;
        }
        return true;
    }

    template<typename T>
    bool NoID(T *r, int index) {
        if (index + 1 == r->p->n_cigar)
            return true;
        if (index == 0)
            return true;

        size_t size = r->p->cigar[index]>>4;
        size_t size_next = r->p->cigar[index + 1]>>4;
        size_t size_prev = r->p->cigar[index - 1]>>4;

        if (index - 1 == 0 && size_prev < 300 && size >= 50)
            return false;

        if (index + 2 == r->p->n_cigar && size_next < 300 && size >= 50)
            return false;

        if ("MIDNSH"[r->p->cigar[index]&0xf] + "MIDNSH"[r->p->cigar[index + 1]&0xf] == 'D' + 'I') {
            return false;
        }
        if ("MIDNSH"[r->p->cigar[index]&0xf] + "MIDNSH"[r->p->cigar[index - 1]&0xf] == 'D' + 'I') {
            return false;
        }
        return true;
    }

    bool CheckAllSame(const std::vector<bool> &v)
    {
        if (v.size() == 0)
            return false;
        return std::all_of(v.begin(), v.end(), [v](int x){ return x==v[0]; });
    }

    void ImproveCigarString(std::vector<std::pair<int, char>> cigar_vector, mm_reg1_t *r) {

    }

    std::vector<Deletion> MergeDeletions(std::vector<Deletion> &dels, const std::string &reference, int win_start) {
        if (dels.size() <= 1)
            return dels;
        std::vector<Deletion> answer;
        answer.push_back(dels[0]);
        for (int i = 1; i < dels.size(); ++i) {
            if (dels[i].ref_position_ - answer.back().second_ref_position_ < 100) {
                Deletion new_del(answer.back().chrom_, answer.back().ref_position_, dels[i].second_ref_position_, answer.back().deletion_seq_ + dels[i].deletion_seq_, reference.substr(answer.back().second_ref_position_  - win_start, dels[i].ref_position_ - answer.back().second_ref_position_));
                answer.pop_back();
                answer.push_back(new_del);
            } else {
                answer.push_back(dels[i]);
            }
        }
        return answer;
    }

    std::vector<Insertion> MergeInsertions(std::vector<Insertion> &ins, const std::string &reference, int win_start) {
        if (ins.size() <= 1)
            return ins;
        std::vector<Insertion> answer;
        answer.push_back(ins[0]);
        for (int i = 1; i < ins.size(); ++i) {
            if (ins[i].ref_position_ - answer.back().ref_position_ < 100) {
                Insertion new_ins(answer.back().chrom_, answer.back().ref_position_, answer.back().insertion_seq_ + reference.substr(answer.back().ref_position_ - win_start, ins[i].ref_position_ - answer.back().ref_position_) + ins[i].insertion_seq_, reference.substr(answer.back().ref_position_ - win_start, ins[i].ref_position_ - answer.back().ref_position_));
                answer.pop_back();
                answer.push_back(new_ins);
            } else {
                answer.push_back(ins[i]);
            }
        }
        return answer;
    }

    void NormalizeDeletion(Deletion &del, const std::string &reference, int reference_start) {
        int temp = reference_start;
        int mistakes = 0;
        bool last_mistake = false;
        while (temp > 0 && temp + del.Size() - 1 < reference.size()) {
            if (reference[temp-1] == reference[temp + del.Size() - 1]) {
                temp--;
                last_mistake = false;
            }
            else {
                mistakes++;
                if (mistakes > 1)
                    break;
                last_mistake = true;
                temp--;
            }
        }
        if (reference_start - temp < 10) {
            return;
        }
        if (last_mistake) {
            temp++;
        }
        del.ref_position_ -= (reference_start - temp);
        del.second_ref_position_ -= (reference_start - temp);
        del.deletion_seq_ = reference.substr(temp, del.Size());
        del.alt_ = reference.substr(temp, 1);
    }


    int RunAndProcessMinimap(const std::string &path_to_scaffolds, const std::string &reference, const std::string &ref_name, int start_pos) {
        //std::string reference_reverse = reference;
        //std::reverse(reference_reverse.begin(), reference_reverse.end());
        const char *reference_cstyle = reference.c_str();
        const char **reference_array = &reference_cstyle;
        mm_idx_t *index = mm_idx_str(10, 19, 0, 14, 1, reference_array, NULL);
        io::FastaFastqGzParser contig_reader(path_to_scaffolds);
        io::SingleRead contig;
        std::set<std::pair<int, int>> found_reference_intervals;
        std::set<omnigraph::MappingRange> found_query_intervals;
        size_t min_contig_size = OptionBase::use_long_reads ? 5000 : 3000;
        int max_hits = 0;
        int contig_num = 0;
        mm_idxopt_t iopt;
        mm_mapopt_t mopt;
        mm_set_opt(0, &iopt, &mopt);
        mm_set_opt("asm20", &iopt, &mopt);
        mopt.flag |= MM_F_CIGAR;
        mopt.bw = 85;
        mm_mapopt_update(&mopt, index);

        while (!contig_reader.eof()) {
            mm_tbuf_t *tbuf = mm_tbuf_init();
            contig_num++;
            if (contig_num == 20)
                break;
            contig_reader >> contig;
            std::string query = contig.GetSequenceString();
            size_t qsize = query.size();
            if (qsize <= min_contig_size)
                continue;
//            std::reverse(query.begin(), query.end());

            int number_of_hits;


//            mopt.zdrop = 500;
//            mopt.zdrop_inv = 10;
//            mopt.b = 5;
//            mopt.q = 4;
//            mopt.q2 = 16;
//            mopt.best_n = 1;
//            mopt.flag |=  MM_F_NO_LJOIN;
//            mopt.flag |= MM_F_SPLICE;
//            mopt.max_gap = 15000;
//          mopt.bw_long = 85;
            mm_reg1_t *hit_array = mm_map(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, contig.name().c_str());
            max_hits = std::max(max_hits, number_of_hits);
            std::vector<bool> is_hit_revcomp;
            std::vector<AlignBlock> all_blocks;
            for (int k = 0; k < std::min(1,number_of_hits); ++k) { // traverse hits and print them out
                mm_reg1_t *r = &hit_array[k];
                printf("%s\t%d\t%d\t%d\t%c\t", contig.name().c_str(), query.size(), r->qs, r->qe, "+-"[r->rev]);
                is_hit_revcomp.push_back(r->rev);

                std::vector<Deletion> deletions;
                std::vector<Insertion> insertions;

                if (r->inv) {
                    ProcessInversion(r, query, ref_name, start_pos);
                }
                else if (!r->rev) {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    std::vector<std::pair<char, int>> cigar_vector;
                    int query_end = query_start;
                    int reference_end = reference_start;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        char ch = "MIDNSH"[r->p->cigar[i]&0xf];
                        int len = (r->p->cigar[i]>>4);

                        if (ch == 'M' || ch == '=' || ch == 'X') {
                            query_end += len;
                            reference_end += len;
                        }
                        if (ch == 'D') {
                            reference_end += len;
                        }
                        if (ch == 'I') {
                            query_end += len;
                        }

                        cigar_vector.push_back({ch, len});
                    }
                    all_blocks.push_back(AlignBlock(reference_start, reference_end, query_start, query_end, cigar_vector));

                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            int temp = (int)(reference_start + int(r->p->cigar[i]>>4));
                            int some_stuff = (r->p->cigar[i])>>4;
                            found_reference_intervals.insert({std::min(reference_start, (int)(reference_start + ((r->p->cigar[i]) >> 4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4)))});
                            found_query_intervals.insert(omnigraph::MappingRange(reference_start, (int)(reference_start + ((r->p->cigar[i]) >> 4)), query_start, query_start + (r->p->cigar[i]>>4)));
                            query_start += (r->p->cigar[i]>>4);
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                            Insertion ins(ref_name, start_pos + reference_start, query.substr(query_start, r->p->cigar[i]>>4), reference.substr(reference_start,1));
                            std::string ins_seq = query.substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    insertions.push_back(ins);
//                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            found_query_intervals.insert(omnigraph::MappingRange(reference_start, (int)(reference_start + ((r->p->cigar[i]) >> 4)), query_start, query_start + (r->p->cigar[i]>>4)));

                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {


                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4), reference.substr(reference_start, 1));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    NormalizeDeletion(del, reference, reference_start);
                                    deletions.push_back(del);
//                                    WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_reference_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                } else {
                    int query_start = r->qs;
                    int reference_start = r->rs;


                    std::vector<std::pair<char, int>> cigar_vector;
                    int query_end = query_start;
                    int reference_end = reference_start;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        char ch = "MIDNSH"[r->p->cigar[i]&0xf];
                        int len = (r->p->cigar[i]>>4);

                        if (ch == 'M' || ch == '=' || ch == 'X') {
                            query_end += len;
                            reference_end += len;
                        }
                        if (ch == 'D') {
                            reference_end += len;
                        }
                        if (ch == 'I') {
                            query_end += len;
                        }

                        cigar_vector.push_back({ch, len});
                    }
                    all_blocks.push_back(AlignBlock(reference_start, reference_end, query_start, query_end, cigar_vector));


                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            found_reference_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4)))});
                            found_query_intervals.insert(omnigraph::MappingRange(reference_start, (int)(reference_start + ((r->p->cigar[i]) >> 4)), query_start, query_start + (r->p->cigar[i]>>4)));
                            query_start += (r->p->cigar[i]>>4);
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {

                            Insertion ins(ref_name, start_pos + reference_start, ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4), reference.substr(reference_start,1));
                            std::string ins_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    insertions.push_back(ins);
//                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            found_query_intervals.insert(omnigraph::MappingRange(reference_start, (int)(reference_start + ((r->p->cigar[i]) >> 4)), query_start, query_start + (r->p->cigar[i]>>4)));
                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4), reference.substr(reference_start, 1));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    NormalizeDeletion(del, reference, reference_start);
                                    deletions.push_back(del);
                                    //WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_reference_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i] >> 4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                }
                free(r->p);
                auto merged_dels = MergeDeletions(deletions, reference, start_pos);
                for (auto del : merged_dels) {

                    WriteCritical(vector_of_del_, del);
                }
                auto merged_ins = MergeInsertions(insertions, reference, start_pos);
                for (auto ins : merged_ins) {
                    WriteCritical(vector_of_ins_, ins);
                }

            }

            free(hit_array);
            mm_tbuf_destroy(tbuf);

//            is_hit_revcomp.clear();
            if (!CheckAllSame(is_hit_revcomp)) {
                found_query_intervals.clear();
                found_reference_intervals.clear();
                continue;
            }
            std::vector<std::pair<int, int>> merged_intervals;
            for (auto p : found_reference_intervals) {
                if (!merged_intervals.size()) {
                    merged_intervals.push_back(p);
                    continue;
                }

                auto last_interval = merged_intervals.back();

                if (p.first > last_interval.second + 50) {
                    if (!is_hit_revcomp[0]) {
                        Deletion del(ref_name, start_pos + last_interval.second, start_pos + p.first, reference.substr(last_interval.second, p.first - last_interval.second), reference.substr(last_interval.second,1));
//                        WriteCritical(vector_of_del_, del);
                    } else {
                        Deletion del(ref_name, start_pos + last_interval.second, start_pos + p.first, reference.substr(last_interval.second, p.first - last_interval.second), reference.substr(last_interval.second, 1));
//                        WriteCritical(vector_of_del_, del);
                    }
                }
                merged_intervals.push_back(p);
            }
            merged_intervals.clear();
            /*
            std::vector<omnigraph::MappingRange> merged_ranges;
            for (auto p : found_query_intervals) {
                if (!merged_ranges.size()) {
                    merged_ranges.push_back(p);
                    continue;
                }

                auto last_interval = merged_ranges.back();

                if (p.mapped_range.start_pos > last_interval.mapped_range.end_pos + 50) {
                    if (!is_hit_revcomp[0]) {
                        Insertion ins(ref_name, start_pos + last_interval.initial_range.end_pos, reference.substr(last_interval.mapped_range.end_pos, p.mapped_range.start_pos - last_interval.mapped_range.end_pos), reference[last_interval.initial_range.end_pos]);
                        WriteCritical(vector_of_ins_, ins);
                    } else {
                        Insertion ins(ref_name, start_pos + last_interval.initial_range.end_pos, ReverseComplement(reference).substr(last_interval.mapped_range.end_pos, p.mapped_range.start_pos - last_interval.mapped_range.end_pos), reference[last_interval.initial_range.end_pos]);
                        WriteCritical(vector_of_ins_, ins);
                    }
                }
                merged_ranges.push_back(p);
            }
            */

            found_query_intervals.clear();
            found_reference_intervals.clear();

        }
        mm_idx_destroy(index);
        return max_hits;
    }



    bool IsGoodRef(const std::string &ref_name) {
        if (ref_name[0] != 'c') {
            return false;
        }
        if (ref_name.size() > 10) {
            return false;
        }
        return true;
    }

    bool IsDegenerate(const std::string &query) {
        std::map<char, int> char_map;
        for (auto ch : query) {
            char_map[ch]++;
        }
        double max_ratio = 0.0;
        for (auto p : char_map) {
            max_ratio = std::max(max_ratio, p.second/double(query.length()));
        }
        return max_ratio > 0.85;
    }

    bool IsBadAlignment(BamTools::BamAlignment &alignment, std::unordered_map<int, std::string> &refid_to_ref_name) {
        //very bad alignment
        for (auto ch : alignment.Qualities) {
            if (ch < '5') {
                return false;
            }
        }

        if (!IsGoodRef(refid_to_ref_name[alignment.RefID])) {
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

        if (alignment.RefID != alignment.MateRefID) {
            return true;
        }

//        std::string tag;
//        alignment.GetTagCore("AM", tag);
//        if (tag == "0") {
//            return true;
//        }
        return false;
    }

    bool IsGoodAlignment(BamTools::BamAlignment &alignment) {
        auto cigar = alignment.CigarData;
        for (auto ch : alignment.Qualities) {
            if (ch < '5') {
                return false;
            }
        }
        if (cigar.size() == 1 && cigar[0].Type == 'M' && alignment.RefID == alignment.MateRefID) {
            return true;
        }
        return false;
    }

    bool OutputPairedRead(BamTools::BamAlignment &alignment, io::OPairedReadStream<std::ofstream, io::FastqWriter> &out_stream, BamTools::BamReader &reader) {

        io::SingleRead first;
        io::SingleRead second;
        //INFO("Jump to " << alignment.MateRefID << " " << alignment.MatePosition);
        if (alignment.IsFirstMate()) {
            std::string read_name = alignment.Name;
            first = CreateRead(alignment);
            VERBOSE_POWER(++jumps, " jumps");
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            int jump_num = 0;
            while(mate_alignment.Position < alignment.MatePosition) {
                ++jump_num;
                if (jump_num > 500) {
                    return false;
                }
                if(!reader.GetNextAlignmentCore(mate_alignment) || mate_alignment.RefID != alignment.MateRefID) {
                    return false;
                }
            }
            mate_alignment.BuildCharData();
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsFirstMate() || !mate_alignment.IsPrimaryAlignment()) {
                if(!reader.GetNextAlignment(mate_alignment)) {
                    return false;
                }
                if (mate_alignment.Position > alignment.MatePosition || mate_alignment.RefID != alignment.MateRefID) {
                    return false;
                }
            }
            second = CreateRead(mate_alignment);
        } else {
            second = CreateRead(alignment);
            std::string read_name = alignment.Name;
            VERBOSE_POWER(++jumps, " jumps");
            //INFO("Jump to " << alignment.MateRefID << " " << alignment.MatePosition);
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            int jump_num = 0;
            while(mate_alignment.Position < alignment.MatePosition) {
                ++jump_num;
                if (jump_num > 500) {
                    return false;
                }
                if(!reader.GetNextAlignmentCore(mate_alignment) || mate_alignment.RefID != alignment.MateRefID) {
                    return false;
                }
            }

            mate_alignment.BuildCharData();
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsSecondMate() || !mate_alignment.IsPrimaryAlignment()) {
                if(!reader.GetNextAlignment(mate_alignment)) {
                    return false;
                }

                if (mate_alignment.Position > alignment.MatePosition || mate_alignment.RefID != alignment.MateRefID) {
                    return false;
                }
            }
            first = CreateRead(mate_alignment);
        }
        io::PairedRead pair(first, second, 0);
        out_stream << pair;
        return true;
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




