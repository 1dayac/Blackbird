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
results. Therefore, our method might decrease the cost of SV calling procedure in clinical
setting, without losing in the result quality.


## Installation

Run 

```
./spades_compile.sh -DSPADES_USE_JEMALLOC=OFF
```

Use blackbird binary from the bin folder.

## Options

## Output Formats

## Example Commands

## Demo command

## Publications


## Contact & Support

Feel free to drop any inquiry to [meleshko.dmitrii@gmail.com]() 