Blackbird: Tool for SV detection with Synthetic Long Reads (SLR) and hybrid (SLR+Long reads datasets)
======
<font size=20>__Blackbird 0.1 Manual__</font>


# Table of contents
1. [About Blackbird](about-blackbird)
2. [Installation](#installation)
3. [Options](#options)
4. [Output Formats](#output-formats)
5. [Example Commands](#example-commands)
6. [Publications](#publications)
7. [Contact & Support](#contact)


## About Blackbird

Blackbird is a novel integrated alignment- and local-assembly-based algorithm hat employs the barcode information encoded in SLR reads to improve
detection and placement of challenging medium-size events (50-10,000bp).
Blackbird assembles the genome into segments and calls insertions and deletions in these segments. Without the need
for a computationally expensive whole genome assembly, Blackbird uses a barcode-aware
sliding window approach to assemble small segments of the target genome and sensitively call
SVs in these segments.

Blackbird is able to work with SLR read datasets and a combination of SLR read
and long-read datasets. We evaluated our method on both simulated and real whole genome
human datasets. In a SLR read mode Blackbird outperforms existing short-read and
SLR read methods, especially for insertions. In a hybrid-mode Blackbird demonstrated
results similar to state-of-the-art long read tools, but requires less long reads to achieve same
results. Therefore, our method might decrease the cost of SV calling in clinical
setting, without losing in the result quality.


## Installation

Initially Blackbird was forked from the SPAdes reposotory and have a common codebase.
To compile Blackbird in assembler foder execute:

```
./spades_compile.sh
```

Blackbird binary can be found inside bin folder. Additionally, you should install and compile [SPAdes for blackbird](https://github.com/1dayac/spades_for_blackbird).

## Options

`--bam` or `-b` [required] - position-sorted and indexed SLR Read bam-file with BX tags

`--rerefence` or `-r` [required] - BWA-indexed reference genome

`--output` or `-o` [required] - Folder where results will be stored

`--spades` or `-s` - Path to spades.py from spades_for_blackbird repository (not needed if spades.py is in path)

`--long-bam` or `-l` - Position-sorted and indexed long read file (optional, should be paired with long-read option)

`--long-read` or `-m` - FASTQ-file with the same long reads (optional, should be paired with long-bam option)

`--threads` or `-t` - Number of threads (default is 1, but more threads is beneficial)

`--regions` or `-r` [optional] - File with regions where SV should be called. Each row in the file is space-separated and has "chr start end" format

`--help` or `-h` - Print this message

## Hidden options

`-d` - Don't delete assembly folders. It allows to inspect contigs that were used for SV calling

`-n` - Don't collect poorly aligned reads

## Output Formats

All files are stored in the output folder which is set by the user.

`out_50.vcf` is a position sorted VCF file, that contains insertion and deletion calls longer than 50 bp.

`out.vcf` contains all other calls, though they are not reliable. 

## Example Commands

`/home/dmm2017/Blackbird/assembler/bin/blackbird -b /local/storage/data/10X/HG002/NA24385.GRCh37.phased_possorted_bam.bam -r /local/workdir/dmm2017/hg37/refdata-hg19-2.1.0/fast
a/genome.fa -o blackbird_10X_chr1 -g genome.txt -t 32 -s /home/dmm2017/spades_for_blackbird/assembler/spades.py`

## Publications

Blackbird is published in [Bioinformatics Advances](https://coursesandconferences.wellcomeconnectingscience.org/event/genome-informatics-20220921/).

## Contact & Support

Feel free to drop any inquiry to [meleshko.dmitrii@gmail.com](mailto:meleshko.dmitrii@gmail.com) 
