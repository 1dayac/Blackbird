//
// Created by Dima on 7/23/19.
//

#ifndef BLACKBIRD_PIPELINE_H
#define BLACKBIRD_PIPELINE_H
#include<algorithm>
#include <list>
#include "utils/logger/log_writers.hpp"
#include "options.h"
#include "io/sam/bam_reader.hpp"
#include "common/io/reads/osequencestream.hpp"
#include <boost/circular_buffer.hpp>
#include <minimap2/minimap.h>
#include "minimap2/minimap.h"
#include "io/reads/fasta_fastq_gz_parser.hpp"
#include "common/utils/parallel/openmp_wrapper.h"
#include "common/utils/memory_limit.hpp"
void create_console_logger(std::string log_prop_fn) {
    using namespace logging;
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



class Deletion {
public:
    Deletion(const std::string &chrom, int ref_position, int second_ref_position, const std::string &deletion_seq)
    : chrom_(chrom), ref_position_(ref_position), deletion_seq_(deletion_seq), second_ref_position_(second_ref_position) {    }

    std::string ToString() const {
        return chrom_ + "\t" +  std::to_string(ref_position_) + "\t<DEL>\t"  + "SEQ=" + deletion_seq_ + ";SVLEN=" + std::to_string(second_ref_position_ - ref_position_) + ";SVTYPE=DEL";
    }

    int Size() const {
        return second_ref_position_ - ref_position_;
    }

    template <class T>
    bool operator < (const T& op2) const
    {
        if (chrom_ < op2.chrom_)
        {
            return true;
        }
        if (chrom_ > op2.chrom_)
        {
            return false;
        }
        if (ref_position_ < op2.ref_position_) {
            return true;
        }
        return false;
    }

    bool operator ==(const Deletion& op2) const {
        return chrom_ == op2.chrom_ && ref_position_ == op2.ref_position_;
    }

    std::string chrom_;
    int ref_position_;
    int second_ref_position_;
    std::string deletion_seq_;
};

class Insertion {
public:
    Insertion(const std::string &chrom, int ref_position, const std::string &insertion_seq)
            : chrom_(chrom), ref_position_(ref_position), insertion_seq_(insertion_seq) {}

    std::string ToString() const {
        return chrom_ + "\t" +  std::to_string(ref_position_) + "\t<INS>\t"  + "SEQ=" + insertion_seq_ + ";SVLEN=" + std::to_string(insertion_seq_.size())  + ";SVTYPE=INS";
    }

    int Size() const {
        return insertion_seq_.size();
    }

    template <class T>
    bool operator < (const T& op2) const
    {
        if (chrom_ < op2.chrom_)
        {
            return true;
        }
        if (chrom_ > op2.chrom_)
        {
            return false;
        }
        if (ref_position_ < op2.ref_position_) {
            return true;
        }
        return false;
    }

    bool operator ==(const Insertion& op2) const {
        return chrom_ == op2.chrom_ && ref_position_ == op2.ref_position_;
    }

    std::string chrom_;
    int ref_position_;
    std::string insertion_seq_;
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

};

class BlackBirdLauncher {



    void test_minimap() {

        BamTools::BamReader reader;
        reader.Open(OptionBase::bam.c_str());
        BamTools::BamRegion region(reader.GetReferenceID("chr13"), 84880000, reader.GetReferenceID("chr13"), 84930000);

        INFO(reference_map_[refid_to_ref_name_[region.RightRefID]].substr(region.LeftPosition, region.RightPosition - region.LeftPosition));
        RunAndProcessMinimap("before_rr.fasta", reference_map_[refid_to_ref_name_[region.RightRefID]].substr(region.LeftPosition, region.RightPosition - region.LeftPosition), "chr13", region.LeftPosition);


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
        mm_reg1_t *hit_array = mm_map(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, name.c_str());
        INFO(hit_array->score);
    }


public:
    BlackBirdLauncher ()
    {}

    int Launch() {
        utils::perf_counter pc;
        std::string log_filename = OptionBase::output_folder + "/blackdird.log";

        fs::make_dir(OptionBase::output_folder);

        writer_ = VCFWriter(OptionBase::output_folder + "/out_50.vcf");
        writer_small_ = VCFWriter(OptionBase::output_folder + "/out.vcf");


        create_console_logger(log_filename);
        INFO("Starting Blackbird");


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
            reference_map_[chrom.name()] = chrom.GetSequenceString();
        }


        INFO("Filter poorly aligned reads");

        BamTools::BamReader reader;
        BamTools::BamReader mate_reader;


        reader.Open(OptionBase::bam.c_str());
        mate_reader.Open(OptionBase::bam.c_str());

        auto ref_data = reader.GetReferenceData();

        for (auto reference : ref_data) {
            refid_to_ref_name_[reader.GetReferenceID(reference.RefName)] = reference.RefName;
        }

        //test_minimap();
        //return 0;

        BamTools::BamAlignment alignment;
        size_t alignment_count = 0;
        size_t alignments_stored = 0;
        int current_refid = -1;
        int current_size = 0;

        while(reader.GetNextAlignment(alignment)) {
            std::string bx;
            VERBOSE_POWER(++alignment_count, " alignments processed");
            alignment.GetTag("BX", bx);
            if (bx == "") {
                continue;
            }
            if (alignment.RefID != current_refid) {
                current_refid = alignment.RefID;
                DEBUG("Processing chromosome " << refid_to_ref_name_[current_refid]);
            }

            if (IsBadAlignment(alignment, refid_to_ref_name_) && alignment.IsPrimaryAlignment()) {
                map_of_bad_reads_[bx].push_back(io::SingleRead(alignment.Name, alignment.QueryBases, alignment.Qualities, io::PhredOffset));
                VERBOSE_POWER(++alignments_stored, " alignments stored");
            }
        }
        INFO("Total " << alignment_count << " alignments processed");
        INFO("Total " << alignments_stored << " alignments stored");
        reader.Close();

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
            r.OpenIndex((OptionBase::bam + ".bai").c_str())
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


        Print(vector_of_ins_, vector_of_del_, writer_);
        Print(vector_of_small_ins_, vector_of_small_del_, writer_small_);

        //test_minimap();

        INFO("Blackbird finished");
        return 0;
    }

private:
    std::unordered_map<std::string, std::list<io::SingleRead>> map_of_bad_reads_;
    VCFWriter writer_;
    VCFWriter writer_small_;
    std::vector<Insertion> vector_of_small_ins_;
    std::vector<Deletion> vector_of_small_del_;
    std::vector<Insertion> vector_of_ins_;
    std::vector<Deletion> vector_of_del_;

    std::unordered_map<std::string, std::string> reference_map_;
    std::unordered_map<int, std::string> refid_to_ref_name_;

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
        for (auto reference : ref_data) {
            //if(target_region.LeftRefID != reader.GetReferenceID(reference.RefName)) {
            //    continue;
            //}
            if (!IsGoodRef(reference.RefName) || !reference_map_.count(reference.RefName)) {
                continue;
            }
            int window_width = 50000;
            int overlap = 10000;
            for (int start_pos = 0; start_pos < reference.RefLength; start_pos += window_width - overlap) {
                //if (start_pos < target_region.LeftPosition || start_pos > target_region.RightPosition || reference.RefName != "chr1") {
                //    continue;
                //}
                RefWindow r(reference.RefName, start_pos, start_pos + window_width);
                reference_windows.push_back(r);
                ++number_of_windows;
            }
        }
        INFO(number_of_windows << " totally created.");
    }

    void ProcessWindow(const RefWindow &window,  BamTools::BamReader &reader, BamTools::BamReader &mate_reader) {
        INFO("Processing " << window.RefName.RefName << " " << window.WindowStart << "-" << window.WindowEnd << " (thread " << omp_get_thread_num() << ")");
        BamTools::BamRegion region(reader.GetReferenceID(window.RefName.RefName), window.WindowStart, reader.GetReferenceID(window.RefName.RefName), window.WindowEnd);
        BamTools::BamAlignment alignment;
        if (!reader.SetRegion(region)) {
            return;
        }
        std::unordered_map<std::string, int> barcodes_count;
        std::set<std::string> barcodes_count_over_threshold_prelim;
        std::set<std::string> barcodes_count_over_threshold;

        const int threshold = 4;
        const int number_of_barcodes_to_assemble = 200;
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
        std::string temp_dir = OptionBase::output_folder + "/" + refid_to_ref_name_[region.RightRefID] + "_" + std::to_string(region.LeftPosition) + "_" + std::to_string(region.RightPosition);
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
            last_entries.push_back(alignment);
            if (last_entries.full() && alignment.Position - last_entries.front().Position < 50) {
                reader.Jump(alignment.RefID, alignment.Position + 500);
                continue;
            }

        }

        for (auto p : filtered_reads) {
            if (p.second.size() == 1) {
                if (alignment.MateRefID == -1) {
                    OutputSingleRead(p.second[0], single_out_stream);
                } else {
                    OutputPairedRead(p.second[0], out_stream, mate_reader);
                }
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




        std::string barcode_file = temp_dir + "/barcodes.txt";
        std::ofstream barcode_output(barcode_file.c_str(), std::ofstream::out);
        for (auto barcode : barcodes_count_over_threshold) {
            barcode_output << barcode << "\n";
        }

        for (auto barcode : barcodes_count_over_threshold) {
            for (auto read : map_of_bad_reads_[barcode]) {
                single_out_stream << read;
            }
        }
        std::string spades_command = OptionBase::path_to_spades + " --cov-cutoff 5 -t 1 --pe1-1 " + temp_dir + "/R1.fastq --pe1-2 " + temp_dir + "/R2.fastq --pe1-s " + temp_dir + "/single.fastq -o  " + temp_dir + "/assembly >/dev/null";
        std::system(spades_command.c_str());
        RunAndProcessMinimap(temp_dir + "/assembly/scaffolds.fasta", reference_map_[refid_to_ref_name_[region.RightRefID]].substr(region.LeftPosition, region.RightPosition - region.LeftPosition), window.RefName.RefName, region.LeftPosition);
        fs::remove_dir(temp_dir.c_str());
    }

    void RunAndProcessMinimap(const std::string &path_to_scaffolds, const std::string &reference, const std::string &ref_name, int start_pos) {
        INFO("Here we will run minimap");
        const char *reference_cstyle = reference.c_str();
        const char **reference_array = &reference_cstyle;
        mm_idx_t *index = mm_idx_str(10, 19, 0, 8, 1, reference_array, NULL);
        INFO("Index built");
        io::FastaFastqGzParser reference_reader(path_to_scaffolds);
        io::SingleRead contig;
        while (!reference_reader.eof()) {
            reference_reader >> contig;
            std::string query = contig.GetSequenceString();
            if (query.size() < 300) {
                continue;
            }
            int number_of_hits;
            mm_tbuf_t *tbuf = mm_tbuf_init();
            mm_idxopt_t iopt;
            mm_mapopt_t mopt;
            mm_set_opt(0, &iopt, &mopt);
            mopt.flag |= MM_F_CIGAR;
            mm_mapopt_update(&mopt, index);
            mm_reg1_t *hit_array = mm_map(index, query.size(), query.c_str(), &number_of_hits, tbuf, &mopt, contig.name().c_str());
            INFO(contig.name().c_str());
            //INFO(hit_array->score);

            for (int j = 0; j < number_of_hits; ++j) { // traverse hits and print them out
                mm_reg1_t *r = &hit_array[j];
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
                            Insertion ins(ref_name, start_pos + reference_start, query.substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = query.substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos) {
                                if (ins.Size() >= 50) {
                                    #pragma omp critical
                                    {
                                        vector_of_ins_.push_back(ins);
                                    }
                                } else {
                                    #pragma omp critical
                                    {
                                        vector_of_small_ins_.push_back(ins);
                                    }
                                }

                            }

                            query_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (del.Size() >= 50) {
                                #pragma omp critical
                                {
                                    vector_of_del_.push_back(del);
                                }
                            } else {
                                #pragma omp critical
                                {

                                    vector_of_small_del_.push_back(del);
                                }
                            }
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

                            Insertion ins(ref_name, start_pos + reference_start, ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4));
                            std::string ins_seq = ReverseComplement(query).substr(query_start, r->p->cigar[i]>>4);
                            if (ins_seq.find("N") == std::string::npos) {

                                if (ins.Size() >= 50) {
                                    #pragma omp critical
                                    {
                                        vector_of_ins_.push_back(ins);
                                    }
                            } else {
                                    #pragma omp critical
                                    {
                                        vector_of_small_ins_.push_back(ins);
                                    }
                            }
                            }
                            query_start += r->p->cigar[i]>>4;
                        }
                        if ("MIDNSH"[r->p->cigar[i]&0xf] == 'D') {
                            Deletion del(ref_name, start_pos + reference_start, start_pos + reference_start + reference.substr(reference_start, r->p->cigar[i]>>4).size(), reference.substr(reference_start, r->p->cigar[i]>>4));
                            if (del.Size() >= 50) {
                                #pragma omp critical
                                {

                                    vector_of_del_.push_back(del);
                                }
                            } else {
                                #pragma omp critical
                                {

                                    vector_of_small_del_.push_back(del);
                                }
                            }
                            reference_start += r->p->cigar[i]>>4;
                        }
                    }// IMPORTANT: this gives the CIGAR in the aligned regions. NO soft/hard clippings!
                }

                //assert(r->p); // with MM_F_CIGAR, this should not be NULL
                //printf("%s\t%d\t%d\t%d\t%c\t\n", contig.name().c_str(), query.size(), r->qs, r->qe, "+-"[r->rev]);
                //printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\tcg:Z:\n", index->seq[r->rid].name, index->seq[r->rid].len, r->rs, r->re, r->mlen, r->blen, r->mapq);
                free(r->p);
            }
            free(hit_array);
            mm_tbuf_destroy(tbuf);
        }

        mm_idx_destroy(index);
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

    void OutputPairedRead(BamTools::BamAlignment &alignment, io::OPairedReadStream<std::ofstream, io::FastqWriter> &out_stream, BamTools::BamReader &reader) {

        io::SingleRead first;
        io::SingleRead second;

        if (alignment.IsFirstMate()) {

            std::string read_name = alignment.Name;
            first = CreateRead(alignment);
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            int jump_num = 0;
            while(mate_alignment.Position < alignment.MatePosition) {
                ++jump_num;
                if (jump_num > 100000) {
                    return;
                }
                if(!reader.GetNextAlignmentCore(mate_alignment) || mate_alignment.RefID != alignment.MateRefID) {
                    return;
                }
            }
            mate_alignment.BuildCharData();
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsFirstMate() || !mate_alignment.IsPrimaryAlignment()) {
                if(!reader.GetNextAlignment(mate_alignment)) {
                    return;
                }
                if (mate_alignment.Position > alignment.MatePosition || mate_alignment.RefID != alignment.MateRefID) {
                    return;
                }
            }
            second = CreateRead(mate_alignment);
        } else {
            second = CreateRead(alignment);
            std::string read_name = alignment.Name;
            reader.Jump(alignment.MateRefID, alignment.MatePosition);
            BamTools::BamAlignment mate_alignment;
            int jump_num = 0;
            while(mate_alignment.Position < alignment.MatePosition) {
                ++jump_num;
                if (jump_num > 100000) {
                    return;
                }
                if(!reader.GetNextAlignmentCore(mate_alignment) || mate_alignment.RefID != alignment.MateRefID) {
                    return;
                }
            }

            mate_alignment.BuildCharData();
            while(mate_alignment.Name != alignment.Name || mate_alignment.IsSecondMate() || !mate_alignment.IsPrimaryAlignment()) {
                if(!reader.GetNextAlignment(mate_alignment)) {
                    return;
                }

                if (mate_alignment.Position > alignment.MatePosition || mate_alignment.RefID != alignment.MateRefID) {
                    return;
                }
            }
            first = CreateRead(mate_alignment);
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

