//
// Created by Dima on 7/23/19.
//

#ifndef BLACKBIRD_PIPELINE_H
#define BLACKBIRD_PIPELINE_H
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
#include <boost/circular_buffer.hpp>
namespace Unimap {
#include <unimap2/unimap.h>
#include "unimap2/unimap.h"
}

namespace Minimap {
#include <minimap2/minimap.h>
}

#include "io/reads/fasta_fastq_gz_parser.hpp"
#include "common/utils/parallel/openmp_wrapper.h"
#include "common/utils/memory_limit.hpp"
#include "svs.h"
void create_console_logger(const std::string& dir, std::string log_prop_fn) {
    using namespace logging;

    if (!fs::FileExists(log_prop_fn))
        log_prop_fn = fs::append_path(dir, log_prop_fn);

    logger *lg = create_logger(fs::FileExists(log_prop_fn) ? log_prop_fn : "");
    lg->add_writer(std::make_shared<console_writer>());
    //lg->add_writer(std::make_shared<mutex_writer>(std::make_shared<console_writer>()));
    attach_logger(lg);
}





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

    std::string ToString() {
        std::string out = RefName.RefName + " " + std::to_string(WindowStart) + " " + std::to_string(WindowEnd);
        return out;
    }
};

class VCFWriter {
    std::ofstream file_;
public:
    VCFWriter() { }

    VCFWriter(const std::string &filename)
    {
       file_ = std::ofstream(filename, std::ofstream::out);
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
        INFO("Inversion");
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
        Inversion inv(ref_name, start_pos + r->rs, start_pos + query_start, inversion_seq);
        WriteCritical(vector_of_inv_, inv);
    }

    void test_minimap(const std::string &path_to_query, const std::string &path_to_reference) {
        io::FastaFastqGzParser reference_reader(path_to_reference);
        std::string reference = "";
        io::SingleRead chrom;
        reference_reader >> chrom;
        reference = chrom.GetSequenceString();
        io::FastaFastqGzParser query_reader(path_to_query);
        while (!query_reader.eof()) {
            query_reader >> chrom;
            std::string query = chrom.GetSequenceString();
            const char *reference_cstyle = reference.c_str();
            const char **reference_array = &reference_cstyle;
            Unimap::mm_idx_t *index = Unimap::mm_idx_str(10, 15, 0, 14, 1, reference_array, NULL);
            mm_idx_stat(index);
            Unimap::mm_tbuf_t *tbuf = Unimap::mm_tbuf_init();
            Unimap::mm_idxopt_t iopt;
            Unimap::mm_mapopt_t mopt;
            int number_of_hits;
            mm_set_opt(0, &iopt, &mopt);
            mm_mapopt_update(&mopt, index);
            mopt.flag |= MM_F_CIGAR;
            std::string name = "123";
            Unimap::mm_reg1_t *hit_array = Unimap::mm_map_seq(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, name.c_str());
            INFO(hit_array->score);
            if (number_of_hits > 0) { // traverse hits and print them out
                Unimap::mm_reg1_t *r = &hit_array[0];
                //printf("%s\t%d\t%d\t%d\t%c\t", contig.name().c_str(), query.size(), r->qs, r->qe, "+-"[r->rev]);
                if (!r->rev) {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        //printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            query_start += r->p->cigar[i]>>4;
                            reference_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                            std::string ins_seq = query.substr(query_start, r->p->cigar[i]>>4);
                            auto t = r->p->cigar[i]>>4;
                            INFO(t);
                            query_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            auto t = r->p->cigar[i]>>4;
                            INFO(t);
                            reference_start += r->p->cigar[i]>>4;
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                } else {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        //printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            query_start += r->p->cigar[i]>>4;
                            reference_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                            auto t = r->p->cigar[i]>>4;
                            INFO(t);
                            std::string ins_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4);
                            query_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            auto t = r->p->cigar[i]>>4;
                            INFO(t);
                            reference_start += r->p->cigar[i]>>4;
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                }
                free(r->p);
            }
            free(hit_array);
            mm_tbuf_destroy(tbuf);
        }
    }

/*    void test_minimap() {

        BamTools::BamReader reader;
        reader.Open(OptionBase::bam.c_str());
        BamTools::BamRegion region(reader.GetReferenceID("chr1"), 80000, reader.GetReferenceID("chr1"), 84930000);

        INFO(reference_map_[refid_to_ref_name_[region.RightRefID]].substr(region.LeftPosition, region.RightPosition - region.LeftPosition));
        RunAndProcessMinimap("contigs.fasta", reference_map_[refid_to_ref_name_[region.RightRefID]].substr(region.LeftPosition, region.RightPosition - region.LeftPosition), "chr13", region.LeftPosition);


        std::string reference = "ACAGAGTTTTCCGATATAGCGTTCTTCTGGCCTCCCCTAATGTTAACATCTTATATAACTACGGTACACTGATCAAAACTAAGAACTTAATATTGAGATGAAGCTATTAACTACTTTACTCAGATTTCACCGGTTTTCCAATGATGTCCTTTTTCTGTTTCAGAATGCAATCCAAGATACCACACTGCATTTAGCTGTACTGTATATGAACACTTTTTAATACATCACTGGCTACAGAATAATAAATTAGTATCGAATCCTATTCTTAAGGATGAGGAACCTGAGTACTGGAGAAGCTAAAGGACTCATCCAGAAGCTCAGTATAAATGAACAATCAGAGTCAGGCCTGTGGTCCTAAAT";
        const char *reference_cstyle = reference.c_str();
        const char **reference_array = &reference_cstyle;
        mm_idx_t *index = mm_idx_str(10, 15, 0, 14, 1, reference_array, NULL);
        mm_idx_stat(index);
        std::string query = "ACAGAGTTTTCCGATATAGCGTTCTTCTGGCCTCCCCTAATGTTAACATCTTATATAACTACGGTACACTGATCAAAACTAAGAACTTGATGTCCTTTTTCTGTTTCAGAATGCAATCCAAGATACCACACTGCATTTAGCTGTACTGTATATGAACACTTTTTAATACATCACTGGCTACAGAATAATAAATTAGTATCGAATCCTATTCTTAAGGATGAGGAACCTGAGTACTGGAGAAGCTAAAGGACTCATCCAGAAGCTCAGTATAAATGAACAATCAGAGTCAGGCCTGTGGTCCTAAAT";
        mm_tbuf_t *tbuf = mm_tbuf_init();
        mm_idxopt_t iopt;
        mm_mapopt_t mopt;
        int number_of_hits;
        mm_set_opt(0, &iopt, &mopt);

        mm_mapopt_update(&mopt, index);
        mopt.flag |= MM_F_CIGAR;
        std::string name = "123";
        mm_reg1_t *hit_array = mm_map_seq(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, name.c_str());
        INFO(hit_array->score);
    }
*/

public:
    BlackBirdLauncher ()
    {}

    int Launch() {
        srand(113018);
        utils::perf_counter pc;
        std::string log_filename = OptionBase::output_folder + "/blackdird.log";

        fs::make_dir(OptionBase::output_folder);

        writer_ = VCFWriter(OptionBase::output_folder + "/out_50.vcf");
        writer_small_ = VCFWriter(OptionBase::output_folder + "/out.vcf");
        writer_inversion_ = VCFWriter(OptionBase::output_folder + "/out_inversions.vcf");

        create_console_logger(OptionBase::output_folder, log_filename);
        INFO("Starting Blackbird");



//        test_minimap("/scratchLocal/dmm2017/temp.fasta", "/scratchLocal/dmm2017/chr1.fasta");
//        return 0;



        int max_treads = omp_get_max_threads();


        if (OptionBase::threads > max_treads) {
            WARN("Only " << max_treads << " thread(s) available.");
            OptionBase::threads = max_treads;
        }

        INFO("Number of threads being used - " << OptionBase::threads);

        INFO("Memory limit  " << (1.0 * (double) utils::get_memory_limit() / 1024 / 1024 / 1024) << " Gb");
        INFO("Free memory - " << utils::get_free_memory());
        INFO("Uploading reference genome");
        utils::limit_memory(utils::get_memory_limit() * 2);
        INFO("Memory limit  " << (1.0 * (double) utils::get_memory_limit() / 1024 / 1024 / 1024) << " Gb");
        io::FastaFastqGzParser reference_reader(OptionBase::reference);
        io::SingleRead chrom;
        while (!reference_reader.eof()) {
            reference_reader >> chrom;
            std::string name = chrom.name().find(" ") == std::string::npos ? chrom.name() : chrom.name().substr(0, chrom.name().find(" "));
            INFO("Adding " << name << " to the map");
            reference_map_[name] = chrom.GetSequenceString();
        }


        BamTools::BamReader reader;
        BamTools::BamReader mate_reader;


        reader.Open(OptionBase::bam.c_str());
        mate_reader.Open(OptionBase::bam.c_str());

        auto ref_data = reader.GetReferenceData();

        for (auto reference : ref_data) {
            refid_to_ref_name_[reader.GetReferenceID(reference.RefName)] = reference.RefName;
        }

        if (!OptionBase::dont_collect_reads) {
            phmap::parallel_flat_hash_map<std::string, std::pair<Sequence, std::string>,
                    phmap::container_internal::hash_default_hash<std::string>,
                    phmap::container_internal::hash_default_eq<std::string>,
                    phmap::container_internal::Allocator<
                            phmap::container_internal::Pair<const std::string, std::pair<Sequence, std::string>>>,
                    4, phmap::NullMutex> map_of_bad_first_reads_;
            phmap::parallel_flat_hash_map<std::string, std::pair<Sequence, std::string>,
                    phmap::container_internal::hash_default_hash<std::string>,
                    phmap::container_internal::hash_default_eq<std::string>,
                    phmap::container_internal::Allocator<
                            phmap::container_internal::Pair<const std::string, std::pair<Sequence, std::string>>>,
                    4, phmap::NullMutex> map_of_bad_second_reads_;

            BamTools::BamAlignment alignment;
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
                    if (alignment.QueryBases.find("N") == std::string::npos) {
                        std::string bx;
                        alignment.GetTag("BX", bx);
                        if (bx == "") {
                            continue;
                        }
                        if (alignment.IsFirstMate())
                            map_of_bad_first_reads_[alignment.Name] = {Sequence(alignment.QueryBases), bx};
                        else
                            map_of_bad_second_reads_[alignment.Name] = {Sequence(alignment.QueryBases), bx};
                        VERBOSE_POWER(++alignments_stored, " alignments stored");
                    }
                }
            }
            for (auto read : map_of_bad_first_reads_) {
                if (map_of_bad_second_reads_.count(read.first)) {
                    map_of_bad_read_pairs_[read.second.second].push_back({read.second.first, map_of_bad_second_reads_[read.first].first});
                } else {
                    map_of_bad_reads_[read.second.second].push_back(read.second.first);
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
        CreateReferenceWindows(reference_windows, ref_data);

        std::vector<BamTools::BamReader> readers(OptionBase::threads);
        std::vector<BamTools::BamReader> mate_readers(OptionBase::threads);
        for (auto &r : readers) {
            r.Open(OptionBase::bam.c_str());
            if (r.OpenIndex((OptionBase::bam + ".bai").c_str())) {
                INFO("Index located at " << OptionBase::bam << ".bai");
            } else {
                FATAL_ERROR("Index at " << OptionBase::bam << ".bai" << " can't be located")
            }

        }

        for (auto &r : mate_readers) {
            r.Open(OptionBase::bam.c_str());
            r.OpenIndex((OptionBase::bam + ".bai").c_str());
        }

        #pragma omp parallel for schedule(dynamic, 1) num_threads(OptionBase::threads)
        for (int i = 0; i < reference_windows.size(); ++i) {
            ProcessWindow(reference_windows[i], readers[omp_get_thread_num()], mate_readers[omp_get_thread_num()]);
            INFO(i << " " << omp_get_thread_num());
        }

        std::sort(vector_of_ins_.begin(), vector_of_ins_.end());
        std::sort(vector_of_del_.begin(), vector_of_del_.end());

        std::sort(vector_of_small_ins_.begin(), vector_of_small_ins_.end());
        std::sort(vector_of_small_del_.begin(), vector_of_small_del_.end());

        std::sort(vector_of_inv_.begin(), vector_of_inv_.end());

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

    void Print(const std::vector<Insertion> &vector_of_ins, const std::vector<Deletion> &vector_of_del, VCFWriter &writer) {
        int i = 0;
        int j = 0;
        while (i != vector_of_ins.size() || j != vector_of_del.size()) {
            if (i == vector_of_ins.size()) {
                writer.Write(vector_of_del[j]);
                j++;
                while(j != vector_of_del.size() && vector_of_del[j] == vector_of_del[j - 1]) {
                    ++j;
                }
                continue;
            }
            if (j == vector_of_del.size()) {
                writer.Write(vector_of_ins[i]);
                ++i;
                while(i != vector_of_ins.size() && vector_of_ins[i] == vector_of_ins[i - 1]) {
                    ++i;
                }
                continue;
            }

            if (vector_of_del[j] < vector_of_ins[i]) {
                writer.Write(vector_of_del[j]);
                j++;
                while(j != vector_of_del.size() && vector_of_del[j] == vector_of_del[j - 1]) {
                    ++j;
                }
            } else {
                writer.Write(vector_of_ins[i]);
                i++;
                while(i != vector_of_ins.size() && vector_of_ins[i] == vector_of_ins[i - 1]) {
                    ++i;
                }
            }
        }
    }


    void CreateReferenceWindows(std::vector<RefWindow> &reference_windows, BamTools::RefVector& ref_data) {
        int number_of_windows = 0;
        int window_width = 50000;
        int overlap = 10000;
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

    void ProcessWindow(const RefWindow &window,  BamTools::BamReader &reader, BamTools::BamReader &mate_reader) {
        INFO("Processing " << window.RefName.RefName << " " << window.WindowStart << "-" << window.WindowEnd << " (thread " << omp_get_thread_num() << ")");
        BamTools::BamRegion region(reader.GetReferenceID(window.RefName.RefName), window.WindowStart, reader.GetReferenceID(window.RefName.RefName), window.WindowEnd);
        BamTools::BamRegion extended_region(reader.GetReferenceID(window.RefName.RefName), std::max(0, (int)window.WindowStart - 1000), reader.GetReferenceID(window.RefName.RefName), window.WindowEnd + 1000);

        BamTools::BamAlignment alignment;
        if (!reader.SetRegion(region)) {
            return;
        }
        std::unordered_map<std::string, int> barcodes_count;
        std::set<std::string> barcodes_count_over_threshold_prelim;
        std::unordered_set<std::string> barcodes_count_over_threshold;

        auto const& const_refid_to_ref_name = refid_to_ref_name_;


        const int threshold = 4;
        const int number_of_barcodes_to_assemble = 3500;
        while(reader.GetNextAlignment(alignment)) {
            if (alignment.IsPrimaryAlignment() && IsGoodAlignment(alignment)) {
                std::string bx = "";
                alignment.GetTag("BX", bx);
                if (bx == "") {
                    continue;
                }
                if (++barcodes_count[bx] > threshold) {
                    barcodes_count_over_threshold_prelim.insert(bx);
                }
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
        boost::circular_buffer<BamTools::BamAlignment> last_entries(100);

        std::unordered_map<std::string, std::vector<BamTools::BamAlignment>> filtered_reads;

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

        int count_add = 0;
        reader.SetRegion(extended_region);
        while (reader.GetNextAlignmentCore(alignment)) {
            if (alignment.Position > extended_region.RightPosition || alignment.RefID != reader.GetReferenceID(window.RefName.RefName)) {
                break;
            }
            if (alignment.Position < region.RightPosition && alignment.Position > region.LeftPosition)
                continue;
            alignment.BuildCharData();
            std::string bx = "";
            alignment.GetTag("BX", bx);
            if (!barcodes_count_over_threshold.count(bx) || !alignment.IsPrimaryAlignment()) {
                continue;
            }
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
            barcode_output << barcode << "\n";
        }

        INFO("Count_add - " << count_add);
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

        std::string spades_command = OptionBase::path_to_spades + " --only-assembler --sc -k 77 -t 1 --pe1-1 " + temp_dir + "/R1.fastq --pe1-2 " + temp_dir + "/R2.fastq --pe1-s " + temp_dir + "/single.fastq -o  " + temp_dir + "/assembly >/dev/null";
        if (!have_singles)
            spades_command = OptionBase::path_to_spades + " --only-assembler --sc -k 77 -t 1 --pe1-1 " + temp_dir + "/R1.fastq --pe1-2 " + temp_dir + "/R2.fastq -o  " + temp_dir + "/assembly >/dev/null";
        std::system(spades_command.c_str());
        auto const& const_reference_map = reference_map_;
        std::string subreference = const_reference_map.at(const_refid_to_ref_name.at(region.RightRefID)).substr(region.LeftPosition, region.RightPosition - region.LeftPosition);
        int hits = RunAndProcessMinimap(temp_dir + "/assembly/contigs.fasta", subreference, window.RefName.RefName, region.LeftPosition);
        if (hits > 1) {
            //RunAndProcessUnimap(temp_dir + "/assembly/contigs.fasta", subreference, window.RefName.RefName, region, temp_dir);
        }
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
        uniq_number++;
        return io::PairedRead(io::SingleRead(std::to_string(uniq_number), seq.str(), std::string(seq.size(), 'J')),
                io::SingleRead(std::to_string(uniq_number), seq2.str(), std::string(seq2.size(), 'J')), 0);
    }


    bool NoID(Unimap::mm_reg1_t *r, int index) {
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

    int RunAndProcessMinimap(const std::string &path_to_scaffolds, const std::string &reference, const std::string &ref_name, int start_pos) {
        const char *reference_cstyle = reference.c_str();
        const char **reference_array = &reference_cstyle;
        Minimap::mm_idx_t2 *index = Minimap::mm_idx_str2(10, 15, 0, 14, 1, reference_array, NULL);
        io::FastaFastqGzParser reference_reader(path_to_scaffolds);
        io::SingleRead contig;
        std::set<std::pair<int, int>> found_intervals;
        int max_hits = 0;
        std::cout << ("Here1");
        while (!reference_reader.eof()) {
            reference_reader >> contig;
            std::string query = contig.GetSequenceString();
            size_t qsize = query.size();

            int number_of_hits;
            Minimap::mm_tbuf_t2 *tbuf = Minimap::mm_tbuf_init2();
            Minimap::mm_idxopt_t2 iopt;
            Minimap::mm_mapopt_t2 mopt;
            std::cout << ("Here2");

            Minimap::mm_set_opt2(0, &iopt, &mopt);
            mopt.flag |= MM_F_CIGAR;
            Minimap::mm_mapopt_update2(&mopt, index);
            Minimap::mm_reg1_t2 *hit_array = mm_map2(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, contig.name().c_str());
            max_hits = std::max(max_hits, number_of_hits);
            std::cout << ("Here3");
            for (int k = 0; k < std::min(1, number_of_hits); ++k) { // traverse hits and print them out
                Minimap::mm_reg1_t2 *r = &hit_array[k];
                printf("%s\t%d\t%d\t%d\t%c\t", contig.name().c_str(), query.size(), r->qs, r->qe, "+-"[r->rev]);
                if (r->inv) {
                    ProcessInversion(r, query, ref_name, start_pos);
                }
                else if (!r->rev) {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i])>>4)), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            query_start += (r->p->cigar[i]>>4);
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                            Insertion ins(ref_name, start_pos + reference_start, query.substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = query.substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                } else {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            query_start += (r->p->cigar[i]>>4);
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {

                            Insertion ins(ref_name, start_pos + reference_start, ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                }
                free(r->p);

            }

            free(hit_array);
            mm_tbuf_destroy2(tbuf);
        }
        std::vector<std::pair<int, int>> merged_intervals;
        for (auto p : found_intervals) {
            if (!merged_intervals.size()) {
                merged_intervals.push_back(p);
                continue;
            }
            auto last_interval = merged_intervals.back();
            if (p.first > last_interval.second) {
                merged_intervals.push_back(p);
                continue;
            }
            if (p.second > last_interval.second) {
                merged_intervals[merged_intervals.size() - 1] = {last_interval.first, p.second};
            }
        }
        mm_idx_destroy2(index);
        return max_hits;
    }

    void RunAndProcessUnimap(const std::string &path_to_scaffolds, const std::string &reference, const std::string &ref_name, const BamTools::BamRegion &region, const std::string &temp_dir) {
        int start_pos = region.LeftPosition;
        io::FastaWriter reference_writer;
        std::string path_to_reference = temp_dir + "/reference.fasta";
        std::ofstream out(path_to_reference);
        io::SingleRead reference_read(std::to_string(region.LeftRefID) + "_" + std::to_string(start_pos), reference);
        reference_writer.Write(out, reference_read);
        Unimap::mm_idx_t *index = Unimap::um_idx_gen(path_to_reference.c_str(), 5, 15, 14, 0, 35, 1000000000, 50, 1);
        io::FastaFastqGzParser reference_reader(path_to_scaffolds);
        io::SingleRead contig;
        std::set<std::pair<int, int>> found_intervals;
        while (!reference_reader.eof()) {
            reference_reader >> contig;
            std::string query = contig.GetSequenceString();
            size_t qsize = query.size();

            int number_of_hits;
            Unimap::mm_tbuf_t *tbuf = Unimap::mm_tbuf_init();
            Unimap::mm_idxopt_t iopt;
            Unimap::mm_mapopt_t mopt;
            Unimap::mm_set_opt(0, &iopt, &mopt);
            mopt.flag |= MM_F_CIGAR;
            Unimap::mm_mapopt_update(&mopt, index);
            Unimap::mm_reg1_t *hit_array = mm_map_seq(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, contig.name().c_str());
            for (int k = 0; k < std::min(1, number_of_hits); ++k) { // traverse hits and print them out
                Unimap::mm_reg1_t *r = &hit_array[k];
                printf("%s\t%d\t%d\t%d\t%c\t", contig.name().c_str(), query.size(), r->qs, r->qe, "+-"[r->rev]);
                if (r->inv) {
                    ProcessInversion(r, query, ref_name, start_pos);
                }
                else if (!r->rev) {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i])>>4)), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            query_start += (r->p->cigar[i]>>4);
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {
                            Insertion ins(ref_name, start_pos + reference_start, query.substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = query.substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                } else {
                    int query_start = r->qs;
                    int reference_start = r->rs;
                    for (int i = 0; i < r->p->n_cigar; ++i) {
                        printf("%d%c", r->p->cigar[i]>>4, "MIDNSH"[r->p->cigar[i]&0xf]);
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'M') {
                            query_start += (r->p->cigar[i]>>4);
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'I') {

                            Insertion ins(ref_name, start_pos + reference_start, ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos && qsize > 5000 && NoID(r, i)) {
                                if (ins.Size() >= 50) {
                                    WriteCritical(vector_of_ins_, ins);
                                } else {
                                    WriteCritical(vector_of_small_ins_, ins);
                                }
                            }
                            query_start += (r->p->cigar[i]>>4);
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (qsize > 5000 && NoID(r, i)) {
                                if (del.Size() >= 50) {
                                    WriteCritical(vector_of_del_, del);
                                } else {
                                    WriteCritical(vector_of_small_del_, del);
                                }
                            }
                            found_intervals.insert({std::min(reference_start, (int)(reference_start + (r->p->cigar[i]>>4))), std::max(reference_start, (int)(reference_start + (r->p->cigar[i]>>4)))});
                            reference_start += (r->p->cigar[i]>>4);
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                }
                free(r->p);

            }

            free(hit_array);
            Unimap::mm_tbuf_destroy(tbuf);
        }
        std::vector<std::pair<int, int>> merged_intervals;
        for (auto p : found_intervals) {
            if (!merged_intervals.size()) {
                merged_intervals.push_back(p);
                continue;
            }
            auto last_interval = merged_intervals.back();
            if (p.first > last_interval.second) {
                merged_intervals.push_back(p);
                continue;
            }
            if (p.second > last_interval.second) {
                merged_intervals[merged_intervals.size() - 1] = {last_interval.first, p.second};
            }
        }
        INFO(merged_intervals);
        for (int i = 0; i < (int)merged_intervals.size() - 1; ++i) {
            if (merged_intervals[i].second + 50 < merged_intervals[i + 1].first) {
                Deletion del(ref_name, start_pos + merged_intervals[i].second, start_pos + merged_intervals[i + 1].first, reference.substr(merged_intervals[i].second, merged_intervals[i + 1].first - merged_intervals[i].second));
                if (del.HasN())
                    continue;
                INFO("Potential deletion - "  << del.ToString());
            }
        }

        Unimap::mm_idx_destroy(index);
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

        int tag;
        alignment.GetTag("AM", tag);
        if (tag - '0' == 0) {
            return true;
        }
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

int BlackBirdLauncher::uniq_number = 1;
int BlackBirdLauncher::jumps = 0;

#endif //BLACKBIRD_PIPELINE_H

