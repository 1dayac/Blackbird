//
// Created by dmm2017 on 11/4/20.
//

#ifndef BLACKBIRD_SVS_H
#define BLACKBIRD_SVS_H


class Inversion {
public:
    Inversion(const std::string &chrom, int ref_position, int second_ref_position, const std::string &inversion_seq)
            : chrom_(chrom), ref_position_(ref_position), inversion_seq_(inversion_seq), second_ref_position_(second_ref_position) {    }
    std::string chrom_;
    int ref_position_;
    int second_ref_position_;
    std::string inversion_seq_;

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

    bool operator ==(const Inversion& op2) const {
        return chrom_ == op2.chrom_ && ref_position_ == op2.ref_position_;
    }

    std::string ToString() const {
        return chrom_ + "\t" +  std::to_string(ref_position_) + "\t<INV>\tPASS\t"  + "SEQ=" + inversion_seq_ + ";SVLEN=" + std::to_string(inversion_seq_.length()) + ";SVTYPE=INV;ENDPOS=" + std::to_string(second_ref_position_);
    }

};

class Deletion {
public:
    Deletion(const std::string &chrom, int ref_position, int second_ref_position, const std::string &deletion_seq)
            : chrom_(chrom), ref_position_(ref_position), deletion_seq_(deletion_seq), second_ref_position_(second_ref_position) {    }

    std::string ToString() const {
        return chrom_ + "\t" +  std::to_string(ref_position_) + "\t<DEL>\tPASS\t"  + "SEQ=" + deletion_seq_ + ";SVLEN=" + std::to_string(second_ref_position_ - ref_position_) + ";SVTYPE=DEL";
    }

    bool HasN() const {
        return (deletion_seq_.find('N') != std::string::npos);
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
        return chrom_ + "\t" +  std::to_string(ref_position_) + "\t<INS>\tPASS\t"  + "SEQ=" + insertion_seq_ + ";SVLEN=" + std::to_string(insertion_seq_.size())  + ";SVTYPE=INS";
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

#endif //BLACKBIRD_SVS_H
