#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bseq.h"
#include "unimap.h"
#include "umpriv.h"
#include "ketopt.h"

#ifdef __linux__
#include <sys/resource.h>
#include <sys/time.h>
void liftrlimit()
{
	struct rlimit r;
	getrlimit(RLIMIT_AS, &r);
	r.rlim_cur = r.rlim_max;
	setrlimit(RLIMIT_AS, &r);
}
#else
void liftrlimit() {}
#endif

static ko_longopt_t long_options[] = {
	{ "bucket-bits",    ko_required_argument, 300 },
	{ "mb-size",        ko_required_argument, 'K' },
	{ "seed",           ko_required_argument, 302 },
	{ "no-kalloc",      ko_no_argument,       303 },
	{ "dbg-qname",      ko_no_argument,       304 },
	{ "no-self",        ko_no_argument,       'D' },
	{ "print-seeds",    ko_no_argument,       306 },
	{ "max-chain-skip", ko_required_argument, 307 },
	{ "min-dp-len",     ko_required_argument, 308 },
	{ "print-aln-seq",  ko_no_argument,       309 },
	{ "splice",         ko_no_argument,       310 },
	{ "rmq",            ko_required_argument, 311 },
	{ "cost-non-gt-ag", ko_required_argument, 'C' },
	{ "secondary",      ko_required_argument, 315 },
	{ "cs",             ko_optional_argument, 316 },
	{ "end-bonus",      ko_required_argument, 317 },
	{ "no-pairing",     ko_no_argument,       318 },
	{ "splice-flank",   ko_required_argument, 319 },
	{ "idx-no-seq",     ko_no_argument,       320 },
	{ "end-seed-pen",   ko_required_argument, 321 },
	{ "for-only",       ko_no_argument,       322 },
	{ "rev-only",       ko_no_argument,       323 },
	{ "all-chain",      ko_no_argument,       'P' },
	{ "dual",           ko_required_argument, 326 },
	{ "max-clip-ratio", ko_required_argument, 327 },
	{ "MD",             ko_no_argument,       329 },
	{ "score-N",        ko_required_argument, 331 },
	{ "eqx",            ko_no_argument,       332 },
	{ "paf-no-hit",     ko_no_argument,       333 },
	{ "no-end-flt",     ko_no_argument,       335 },
	{ "hard-mask-level",ko_no_argument,       336 },
	{ "cap-sw-mem",     ko_required_argument, 337 },
	{ "max-qlen",       ko_required_argument, 338 },
	{ "max-chain-iter", ko_required_argument, 339 },
	{ "junc-bed",       ko_required_argument, 340 },
	{ "junc-bonus",     ko_required_argument, 341 },
	{ "sam-hit-only",   ko_no_argument,       342 },
	{ "chain-gap-scale",ko_required_argument, 343 },
	{ "alt",            ko_required_argument, 344 },
	{ "alt-drop",       ko_required_argument, 345 },
	{ "mask-len",       ko_required_argument, 346 },
	{ "help",           ko_no_argument,       'h' },
	{ "max-intron-len", ko_required_argument, 'G' },
	{ "version",        ko_no_argument,       'V' },
	{ "min-count",      ko_required_argument, 'n' },
	{ "min-chain-score",ko_required_argument, 'm' },
	{ "mask-level",     ko_required_argument, 'M' },
	{ "min-dp-score",   ko_required_argument, 's' },
	{ "sam",            ko_no_argument,       'a' },
	{ 0, 0, 0 }
};

static inline int64_t mm_parse_num(const char *str)
{
	double x;
	char *p;
	x = strtod(str, &p);
	if (*p == 'G' || *p == 'g') x *= 1e9;
	else if (*p == 'M' || *p == 'm') x *= 1e6;
	else if (*p == 'K' || *p == 'k') x *= 1e3;
	return (int64_t)(x + .499);
}

static inline void yes_or_no(mm_mapopt_t *opt, int flag, int long_idx, const char *arg, int yes_to_set)
{
	if (yes_to_set) {
		if (strcmp(arg, "yes") == 0 || strcmp(arg, "y") == 0) opt->flag |= flag;
		else if (strcmp(arg, "no") == 0 || strcmp(arg, "n") == 0) opt->flag &= ~flag;
		else fprintf(stderr, "[WARNING]\033[1;31m option '--%s' only accepts 'yes' or 'no'.\033[0m\n", long_options[long_idx].name);
	} else {
		if (strcmp(arg, "yes") == 0 || strcmp(arg, "y") == 0) opt->flag &= ~flag;
		else if (strcmp(arg, "no") == 0 || strcmp(arg, "n") == 0) opt->flag |= flag;
		else fprintf(stderr, "[WARNING]\033[1;31m option '--%s' only accepts 'yes' or 'no'.\033[0m\n", long_options[long_idx].name);
	}
}

int main(int argc, char *argv[])
{
	const char *opt_str = "2aSDw:k:K:t:r:f:Vv:g:G:d:Xs:x:Hcp:M:n:z:A:B:O:E:m:N:Qu:R:hb:LC:yYPo:F:UW:";
	ketopt_t o = KETOPT_INIT;
	mm_mapopt_t opt;
	mm_idxopt_t ipt;
	int i, c, ret, n_threads = 3, old_best_n = -1;
	int64_t is_idx;
	char *fnw = 0, *rg = 0, *junc_bed = 0, *s, *alt_list = 0;
	FILE *fp_help = stderr;
	mm_idx_t *mi;

	mm_verbose = 3;
	liftrlimit();
	mm_realtime0 = realtime();
	mm_set_opt(0, &ipt, &opt);

	while ((c = ketopt(&o, argc, argv, 1, opt_str, long_options)) >= 0) { // test command line options and apply option -x/preset first
		if (c == 'x') {
			if (mm_set_opt(o.arg, &ipt, &opt) < 0) {
				fprintf(stderr, "[ERROR] unknown preset '%s'\n", o.arg);
				return 1;
			}
		} else if (c == ':') {
			fprintf(stderr, "[ERROR] missing option argument\n");
			return 1;
		} else if (c == '?') {
			fprintf(stderr, "[ERROR] unknown option in \"%s\"\n", argv[o.i - 1]);
			return 1;
		}
	}
	o = KETOPT_INIT;

	while ((c = ketopt(&o, argc, argv, 1, opt_str, long_options)) >= 0) {
		if (c == 'w') ipt.w = atoi(o.arg);
		else if (c == 'k') ipt.k = atoi(o.arg);
		else if (c == 'H') ipt.flag |= MM_I_HPC;
		else if (c == 'U') ipt.flag |= MM_I_NO_DUPIDX;
		else if (c == 'b') ipt.bf_bits = atoi(o.arg);
		else if (c == 'F') ipt.high_occ = mm_parse_num(o.arg);
		else if (c == 'd') fnw = o.arg; // the above are indexing related options, except -I
		else if (c == 'r') opt.bw = (int)mm_parse_num(o.arg);
		else if (c == 't') n_threads = atoi(o.arg);
		else if (c == 'v') mm_verbose = atoi(o.arg);
		else if (c == 'g') opt.max_gap = (int)mm_parse_num(o.arg);
		else if (c == 'G') mm_mapopt_max_intron_len(&opt, (int)mm_parse_num(o.arg));
		else if (c == 'W') opt.adap_dist = atoi(o.arg);
		else if (c == 'M') opt.mask_level = atof(o.arg);
		else if (c == 'c') opt.flag |= MM_F_OUT_CG | MM_F_CIGAR;
		else if (c == 'D') opt.flag |= MM_F_NO_DIAG;
		else if (c == 'P') opt.flag |= MM_F_ALL_CHAINS;
		else if (c == 'X') opt.flag |= MM_F_ALL_CHAINS | MM_F_NO_DIAG | MM_F_NO_DUAL; // -D -P --dual=no
		else if (c == 'a') opt.flag |= MM_F_OUT_SAM | MM_F_CIGAR;
		else if (c == 'Q') opt.flag |= MM_F_NO_QUAL;
		else if (c == 'Y') opt.flag |= MM_F_SOFTCLIP;
		else if (c == 'L') opt.flag |= MM_F_LONG_CIGAR;
		else if (c == 'y') opt.flag |= MM_F_COPY_COMMENT;
		else if (c == 'n') opt.min_cnt = atoi(o.arg);
		else if (c == 'm') opt.min_chain_score = atoi(o.arg);
		else if (c == 'A') opt.a = atoi(o.arg);
		else if (c == 'B') opt.b = atoi(o.arg);
		else if (c == 's') opt.min_dp_max = atoi(o.arg);
		else if (c == 'C') opt.noncan = atoi(o.arg);
		else if (c == 'K') opt.mini_batch_size = mm_parse_num(o.arg);
		else if (c == 'R') rg = o.arg;
		else if (c == 'h') fp_help = stdout;
		else if (c == '2') opt.flag |= MM_F_2_IO_THREADS;
		else if (c == 'f') opt.max_occ = mm_parse_num(o.arg);
		else if (c == 'p') {
			opt.pri_ratio = strtod(o.arg, &s);
			if (*s == ',') opt.pri_ratio_max = strtod(s + 1, &s);
			if (opt.pri_ratio_max < opt.pri_ratio) opt.pri_ratio_max = opt.pri_ratio;
		} else if (c == 'N') {
			old_best_n = opt.best_n;
			opt.best_n = strtol(o.arg, &s, 10);
			if (*s == ',') opt.best_n_max = strtol(s + 1, &s, 10);
			if (opt.best_n_max < opt.best_n) opt.best_n_max = opt.best_n;
		} else if (c == 'o') {
			if (strcmp(o.arg, "-") != 0) {
				if (freopen(o.arg, "wb", stdout) == NULL) {
					fprintf(stderr, "[ERROR]\033[1;31m failed to write the output to file '%s'\033[0m: %s\n", o.arg, strerror(errno));
					exit(1);
				}
			}
		}
		else if (c == 300) ipt.bucket_bits = atoi(o.arg); // --bucket-bits
		else if (c == 302) opt.seed = atoi(o.arg); // --seed
		else if (c == 303) mm_dbg_flag |= MM_DBG_NO_KALLOC; // --no-kalloc
		else if (c == 304) mm_dbg_flag |= MM_DBG_PRINT_QNAME; // --print-qname
		else if (c == 306) mm_dbg_flag |= MM_DBG_PRINT_QNAME | MM_DBG_PRINT_SEED, n_threads = 1; // --print-seed
		else if (c == 307) opt.max_chain_skip = atoi(o.arg); // --max-chain-skip
		else if (c == 339) opt.max_chain_iter = atoi(o.arg); // --max-chain-iter
		else if (c == 308) opt.min_ksw_len = atoi(o.arg); // --min-dp-len
		else if (c == 309) mm_dbg_flag |= MM_DBG_PRINT_QNAME | MM_DBG_PRINT_ALN_SEQ, n_threads = 1; // --print-aln-seq
		else if (c == 310) opt.flag |= MM_F_SPLICE | MM_F_NO_RMQ; // --splice
		else if (c == 311) yes_or_no(&opt, MM_F_NO_RMQ, o.longidx, o.arg, 0); // --rmq
		else if (c == 317) opt.end_bonus = atoi(o.arg); // --end-bonus
		else if (c == 318) opt.flag |= MM_F_INDEPEND_SEG; // --no-pairing
		else if (c == 320) ipt.flag |= MM_I_NO_SEQ; // --idx-no-seq
		else if (c == 321) opt.anchor_ext_shift = atoi(o.arg); // --end-seed-pen
		else if (c == 322) opt.flag |= MM_F_FOR_ONLY; // --for-only
		else if (c == 323) opt.flag |= MM_F_REV_ONLY; // --rev-only
		else if (c == 327) opt.max_clip_ratio = atof(o.arg); // --max-clip-ratio
		else if (c == 329) opt.flag |= MM_F_OUT_MD; // --MD
		else if (c == 331) opt.sc_ambi = atoi(o.arg); // --score-N
		else if (c == 332) opt.flag |= MM_F_EQX; // --eqx
		else if (c == 333) opt.flag |= MM_F_PAF_NO_HIT; // --paf-no-hit
		else if (c == 335) opt.flag |= MM_F_NO_END_FLT; // --no-end-flt
		else if (c == 336) opt.flag |= MM_F_HARD_MLEVEL; // --hard-mask-level
		else if (c == 337) opt.max_sw_mat = mm_parse_num(o.arg); // --cap-sw-mat
		else if (c == 338) opt.max_qlen = mm_parse_num(o.arg); // --max-qlen
		else if (c == 340) junc_bed = o.arg; // --junc-bed
		else if (c == 341) opt.junc_bonus = atoi(o.arg); // --junc-bonus
		else if (c == 342) opt.flag |= MM_F_SAM_HIT_ONLY; // --sam-hit-only
		else if (c == 343) opt.chain_gap_scale = atof(o.arg); // --chain-gap-scale
		else if (c == 344) alt_list = o.arg; // --alt
		else if (c == 345) opt.alt_drop = atof(o.arg); // --alt-drop
		else if (c == 346) opt.mask_len = mm_parse_num(o.arg); // --mask-len
		else if (c == 315) { // --secondary
			yes_or_no(&opt, MM_F_NO_PRINT_2ND, o.longidx, o.arg, 0);
		} else if (c == 316) { // --cs
			opt.flag |= MM_F_OUT_CS | MM_F_CIGAR;
			if (o.arg == 0 || strcmp(o.arg, "short") == 0) {
				opt.flag &= ~MM_F_OUT_CS_LONG;
			} else if (strcmp(o.arg, "long") == 0) {
				opt.flag |= MM_F_OUT_CS_LONG;
			} else if (strcmp(o.arg, "none") == 0) {
				opt.flag &= ~MM_F_OUT_CS;
			} else if (mm_verbose >= 2) {
				fprintf(stderr, "[WARNING]\033[1;31m --cs only takes 'short' or 'long'. Invalid values are assumed to be 'short'.\033[0m\n");
			}
		} else if (c == 319) { // --splice-flank
			yes_or_no(&opt, MM_F_SPLICE_FLANK, o.longidx, o.arg, 1);
		} else if (c == 326) { // --dual
			yes_or_no(&opt, MM_F_NO_DUAL, o.longidx, o.arg, 0);
		} else if (c == 'S') {
			opt.flag |= MM_F_OUT_CS | MM_F_CIGAR | MM_F_OUT_CS_LONG;
			if (mm_verbose >= 2)
				fprintf(stderr, "[WARNING]\033[1;31m option -S is deprecated and may be removed in future. Please use --cs=long instead.\033[0m\n");
		} else if (c == 'V') {
			puts(UM_VERSION);
			return 0;
		} else if (c == 'u') {
			if (*o.arg == 'b') opt.flag |= MM_F_SPLICE_FOR|MM_F_SPLICE_REV; // both strands
			else if (*o.arg == 'f') opt.flag |= MM_F_SPLICE_FOR, opt.flag &= ~MM_F_SPLICE_REV; // match GT-AG
			else if (*o.arg == 'r') opt.flag |= MM_F_SPLICE_REV, opt.flag &= ~MM_F_SPLICE_FOR; // match CT-AC (reverse complement of GT-AG)
			else if (*o.arg == 'n') opt.flag &= ~(MM_F_SPLICE_FOR|MM_F_SPLICE_REV); // don't try to match the GT-AG signal
			else {
				fprintf(stderr, "[ERROR]\033[1;31m unrecognized cDNA direction\033[0m\n");
				return 1;
			}
		} else if (c == 'z') {
			opt.zdrop = opt.zdrop_inv = strtol(o.arg, &s, 10);
			if (*s == ',') opt.zdrop_inv = strtol(s + 1, &s, 10);
		} else if (c == 'O') {
			opt.q = opt.q2 = strtol(o.arg, &s, 10);
			if (*s == ',') opt.q2 = strtol(s + 1, &s, 10);
		} else if (c == 'E') {
			opt.e = opt.e2 = strtol(o.arg, &s, 10);
			if (*s == ',') opt.e2 = strtol(s + 1, &s, 10);
		}
	}
	if (!fnw && !(opt.flag&MM_F_CIGAR))
		ipt.flag |= MM_I_NO_SEQ;
	if (mm_check_opt(&ipt, &opt) < 0)
		return 1;
	if (opt.best_n == 0) {
		fprintf(stderr, "[WARNING]\033[1;31m changed '-N 0' to '-N %d --secondary=no'.\033[0m\n", old_best_n);
		opt.best_n = old_best_n, opt.flag |= MM_F_NO_PRINT_2ND;
	}

	if (argc == o.ind || fp_help == stdout) {
		fprintf(fp_help, "Usage: unimap [options] <target.fa>|<target.idx> [query.fa] [...]\n");
		fprintf(fp_help, "Options:\n");
		fprintf(fp_help, "  Indexing:\n");
		fprintf(fp_help, "    -k INT       k-mer size (no larger than 28) [%d]\n", ipt.k);
		fprintf(fp_help, "    -w INT       minimizer window size [%d]\n", ipt.w);
		fprintf(fp_help, "    -b INT       number of bits for bloom filter [%d]\n", ipt.bf_bits);
		fprintf(fp_help, "    -d FILE      dump index to FILE []\n");
		fprintf(fp_help, "    -F INT       high k-mer occurrence threshold when indexing [%d]\n", ipt.high_occ);
		fprintf(fp_help, "  Mapping:\n");
		fprintf(fp_help, "    -f INT       max minimizer occurrence [%d]\n", opt.max_occ);
		fprintf(fp_help, "    -g NUM       stop chain enlongation if there are no minimizers in INT-bp [%d]\n", opt.max_gap);
		fprintf(fp_help, "    -G NUM       max intron length (effective with -xsplice; changing -r) [200k]\n");
		fprintf(fp_help, "    -r NUM       bandwidth used in chaining and DP-based alignment [%d]\n", opt.bw);
		fprintf(fp_help, "    -n INT       minimal number of minimizers on a chain [%d]\n", opt.min_cnt);
		fprintf(fp_help, "    -m INT       minimal chaining score (matching bases minus log gap penalty) [%d]\n", opt.min_chain_score);
		fprintf(fp_help, "    -p FLOAT     min secondary-to-primary score ratio [%g]\n", opt.pri_ratio);
		fprintf(fp_help, "    -N INT[,INT] retain at most INT secondary alignments [%d,%d]\n", opt.best_n, opt.best_n_max);
		fprintf(fp_help, "  Alignment:\n");
		fprintf(fp_help, "    -A INT       matching score [%d]\n", opt.a);
		fprintf(fp_help, "    -B INT       mismatch penalty [%d]\n", opt.b);
		fprintf(fp_help, "    -O INT[,INT] gap open penalty [%d,%d]\n", opt.q, opt.q2);
		fprintf(fp_help, "    -E INT[,INT] gap extension penalty; a k-long gap costs min{O1+k*E1,O2+k*E2} [%d,%d]\n", opt.e, opt.e2);
		fprintf(fp_help, "    -z INT[,INT] Z-drop score and inversion Z-drop score [%d,%d]\n", opt.zdrop, opt.zdrop_inv);
		fprintf(fp_help, "    -s INT       minimal peak DP alignment score [%d]\n", opt.min_dp_max);
		fprintf(fp_help, "    -u CHAR      how to find GT-AG. f:transcript strand, b:both strands, n:don't match GT-AG [n]\n");
		fprintf(fp_help, "  Input/Output:\n");
		fprintf(fp_help, "    -c           perform base-alignment and output CIGAR in the PAF format\n");
		fprintf(fp_help, "    -a           perform base-alignment and output in SAM (PAF by default)\n");
		fprintf(fp_help, "    -o FILE      output alignments to FILE [stdout]\n");
		fprintf(fp_help, "    -R STR       SAM read group line in a format like '@RG\\tID:foo\\tSM:bar' []\n");
		fprintf(fp_help, "    --cs[=STR]   output the cs tag; STR is 'short' (if absent) or 'long' [none]\n");
		fprintf(fp_help, "    --MD         output the MD tag\n");
		fprintf(fp_help, "    -Y           use soft clipping for supplementary alignments\n");
		fprintf(fp_help, "    -t INT       number of threads [%d]\n", n_threads);
		fprintf(fp_help, "    -K NUM       minibatch size for mapping [1G]\n");
//		fprintf(fp_help, "    -v INT       verbose level [%d]\n", mm_verbose);
		fprintf(fp_help, "    --version    show version number\n");
		fprintf(fp_help, "  Preset:\n");
		fprintf(fp_help, "    -x STR       preset (always applied before other options; see unimap.1 for details) []\n");
		fprintf(fp_help, "                 - ont/clr:  --rmq=no -r10k -A2 -B4 -O4,24 -E2,1 -z400,200 -s80 -K500M\n");
		fprintf(fp_help, "                 - hifi/ccs: --rmq=no -r10k -A1 -B4 -O6,26 -E2,1 -w21 -K500M\n");
		fprintf(fp_help, "                 - asm5:  -A1 -B19 -O39,81 -E2,1 -N50 -w21\n");
		fprintf(fp_help, "                 - asm10: -A1 -B9  -O16,41 -E2,1 -N50 -w21\n");
		fprintf(fp_help, "                 - asm20: -A1 -B4  -O6,26  -E2,1 -N50\n");
		fprintf(fp_help, "                 - splice:hq/splice: spliced alignment\n");
		fprintf(fp_help, "\nSee `man ./unimap.1' for detailed description of these and other advanced command-line options.\n");
		return fp_help == stdout? 0 : 1;
	}

	if (opt.best_n == 0 && (opt.flag&MM_F_CIGAR) && mm_verbose >= 2)
		fprintf(stderr, "[WARNING]\033[1;31m `-N 0' reduces alignment accuracy. Please use --secondary=no to suppress secondary alignments.\033[0m\n");

	is_idx = mm_idx_is_idx(argv[o.ind]);
	if (is_idx < 0) {
		fprintf(stderr, "[ERROR] failed to open file '%s': %s\n", argv[o.ind], strerror(errno));
		return 1;
	} else if (is_idx) {
		FILE *fp;
		fp = fopen(argv[o.ind], "rb");
		mi = mm_idx_load(fp);
		fclose(fp);
		if (mi->k != ipt.k || mi->w != ipt.w || mi->high_occ != ipt.high_occ || mi->flag != ipt.flag)
			fprintf(stderr, "[WARNING]\033[1;31m indexing parameters from the command line are overriden by `%s'\033[0m\n", argv[o.ind]);
	} else {
		if (fnw == 0 && argc - o.ind < 2) {
			fprintf(stderr, "[ERROR] missing input: please specify a query file to map or option -d to keep the index\n");
			return 1;
		}
		mi = um_idx_gen(argv[o.ind], ipt.w, ipt.k, ipt.bucket_bits, ipt.flag, ipt.bf_bits, ipt.mini_batch_size, ipt.high_occ, n_threads);
		if (fnw) {
			FILE *fp;
			if ((fp = fopen(fnw, "wb")) == 0) {
				fprintf(stderr, "[ERROR] failed to write the index to file '%s'\n", fnw);
				return 1;
			}
			mm_idx_dump(fp, mi);
			fclose(fp);
		}
	}

	if ((opt.flag & MM_F_CIGAR) && (mi->flag & MM_I_NO_SEQ)) {
		fprintf(stderr, "[ERROR] the prebuilt index doesn't contain sequences.\n");
		mm_idx_destroy(mi);
		return 1;
	}
	if (opt.flag & MM_F_OUT_SAM) {
		ret = mm_write_sam_hdr(mi, rg, UM_VERSION, argc, argv);
		if (ret != 0) {
			mm_idx_destroy(mi);
			return 1;
		}
	}
	if (mm_verbose >= 3)
		fprintf(stderr, "[M::%s::%.3f*%.2f] loaded/built the index for %d target sequence(s)\n",
				__func__, realtime() - mm_realtime0, cputime() / (realtime() - mm_realtime0), mi->n_seq);
	if (argc != o.ind + 1) mm_mapopt_update(&opt, mi);
	if (mm_verbose >= 3) mm_idx_stat(mi);
	if (junc_bed) mm_idx_bed_read(mi, junc_bed, 1);
	if (alt_list) mm_idx_alt_read(mi, alt_list);

	ret = 0;
	for (i = o.ind + 1; i < argc; ++i) {
		ret = mm_map_file(mi, argv[i], &opt, n_threads);
		if (ret < 0) break;
	}
	mm_idx_destroy(mi);
	if (ret < 0) {
		fprintf(stderr, "ERROR: failed to map the query file\n");
		exit(EXIT_FAILURE);
	}

	if (fflush(stdout) == EOF) {
		perror("[ERROR] failed to write the results");
		exit(EXIT_FAILURE);
	}

	if (mm_verbose >= 3) {
		fprintf(stderr, "[M::%s] Version: %s\n", __func__, UM_VERSION);
		fprintf(stderr, "[M::%s] CMD:", __func__);
		for (i = 0; i < argc; ++i)
			fprintf(stderr, " %s", argv[i]);
		fprintf(stderr, "\n[M::%s] Real time: %.3f sec; CPU: %.3f sec; Peak RSS: %.3f GB\n", __func__, realtime() - mm_realtime0, cputime(), peakrss() / 1024.0 / 1024.0 / 1024.0);
	}
	return 0;
}
