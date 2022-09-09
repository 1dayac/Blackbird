Blackbird: Tool for SV detection with Linked-Reads reads and hybrid (Linked+Long reads datasets)
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

Blackbird is a novel integrated alignment- and local-assembly-based algorithm hat employs the barcode information encoded in Linked-reads to improve
detection and placement of challenging medium-size events (50-10,000bp).
Blackbird assembles the genome into segments and calls insertions and deletions in these segments. Without the need
for a computationally expensive whole genome assembly, Blackbird uses a barcode-aware
sliding window approach to assemble small segments of the target genome and sensitively call
SVs in these segments.

Blackbird is able to work with Linked-read datasets and a combination of Linked-read
and long-read datasets. We evaluated our method on both simulated and real whole genome
human datasets. In a Linked-read mode Blackbird outperforms existing short-read and
Linked-read methods, especially for insertions. In a hybrid-mode Blackbird demonstrated
results similar to state-of-the-art long read tools, but requires less long reads to achieve same
results. Therefore, our method might decrease the cost of SV calling in clinical
setting, without losing in the result quality.


## Installation

Initially Blackbird was forked from the SPAdes reposotory and have a common codebase.
To compile Blackbird in assembler foder execute:

```
./spades_compile.sh
```

Blackbird binary can be found inside bin folder.

## Options

`--bam` or `-b` [required] - position-sorted and indexed Linked-Read bam-file with BX tags

`--rerefence` or `-r` [required] - BWA-indexed reference genome

`--output` or `-o` [required] - Folder where results will be stored

`--spades` or `-s` - Path to spades.py from spades_for_blackbird repository (not needed if spades.py is in path)

`--long-bam` or `-l` - Position-sorted and indexed long read file (optional, should be paired with long-read option)

`--long-read` or `-m` - FASTQ-file with the same long reads (optional, should be paired with long-bam option)

`--threads` or `-t` - Number of threads (default is 1, but more threads is beneficial)

`--regions` or `-r` [optional] - File with regions where SV should be called. Each row in the file is space-separated and has "chr start end" format

`--help` or `-h` - Print this message


## Output Formats

## Example Commands

## Demo command

## Publications


## Contact & Support

Feel free to drop any inquiry to [meleshko.dmitrii@gmail.com]() 