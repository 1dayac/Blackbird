#ifndef UMPRIV_H
#define UMPRIV_H

#include <assert.h>
#include "unimap.h"
#include "bseq.h"
#include "kseq.h"

#define MM_PARENT_UNSET   (-1)
#define MM_PARENT_TMP_PRI (-2)

#define MM_DBG_NO_KALLOC     0x1
#define MM_DBG_PRINT_QNAME   0x2
#define MM_DBG_PRINT_SEED    0x4
#define MM_DBG_PRINT_ALN_SEQ 0x8

#define MM_SEED_LONG_JOIN  (1ULL<<40)
#define MM_SEED_IGNORE     (1ULL<<41)
#define MM_SEED_TANDEM     (1ULL<<42)
#define MM_SEED_SELF       (1ULL<<43)

#define MM_SEED_SEG_SHIFT  48
#define MM_SEED_SEG_MASK   (0xffULL<<(MM_SEED_SEG_SHIFT))

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

#define mm_seq4_set(s, i, c) ((s)[(i)>>3] |= (uint32_t)(c) << (((i)&7)<<2))
#define mm_seq4_get(s, i)    ((s)[(i)>>3] >> (((i)&7)<<2) & 0xf)

#define MALLOC(type, len) ((type*)malloc((len) * sizeof(type)))
#define CALLOC(type, len) ((type*)calloc((len), sizeof(type)))
#define REALLOC(type, p, len) ((type*)realloc((p), (len) * sizeof(type)))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int n_u, n_a;
	uint64_t *u;
	mm128_t *a;
} mm_seg_t;

double cputime(void);
double realtime(void);
long peakrss(void);

void radix_sort_128x(mm128_t *beg, mm128_t *end);
void radix_sort_64(uint64_t *beg, uint64_t *end);
uint32_t ks_ksmall_uint32_t(size_t n, uint32_t arr[], size_t kk);

void mm_sketch(void *km, const char *str, int len, int w, int k, uint32_t rid, int is_hpc, mm128_v *p, const void *di, int skip_bf, int adap_occ, int adap_dist);

int mm_write_sam_hdr(const mm_idx_t *mi, const char *rg, const char *ver, int argc, char *argv[]);
void mm_write_paf(kstring_t *s, const mm_idx_t *mi, const mm_bseq1_t *t, const mm_reg1_t *r, void *km, int opt_flag, int rep_len);
void mm_write_sam(kstring_t *s, const mm_idx_t *mi, const mm_bseq1_t *t, int reg_idx, int n_reg, const mm_reg1_t *regs, void *km, int opt_flag, int rep_len);

void mm_idxopt_init(mm_idxopt_t *opt);
const uint64_t *mm_idx_get(const mm_idx_t *mi, uint64_t minier, int *n);
mm_reg1_t *mm_align_skeleton(void *km, const mm_mapopt_t *opt, const mm_idx_t *mi, int qlen, const char *qstr, int *n_regs_, mm_reg1_t *regs, mm128_t *a);

mm128_t *mg_lchain_dp(int max_dist_x, int max_dist_y, int bw, int max_skip, int max_iter, int min_cnt, int min_sc, float chn_pen_gap, float chn_pen_skip,
					  int is_cdna, int64_t n, mm128_t *a, int *n_u_, uint64_t **_u, void *km);
mm128_t *mg_lchain_rmq(int max_dist, int max_dist_inner, int bw, int max_chn_skip, int cap_rmq_size, int min_cnt, int min_sc, float chn_pen_gap, float chn_pen_skip,
					   int64_t n, mm128_t *a, int *n_u_, uint64_t **_u, void *km);

mm_reg1_t *mm_gen_regs(void *km, uint32_t hash, int qlen, int n_u, uint64_t *u, mm128_t *a);
void mm_mark_alt(const mm_idx_t *mi, int n, mm_reg1_t *r);
void mm_split_reg(mm_reg1_t *r, mm_reg1_t *r2, int n, int qlen, mm128_t *a);
void mm_sync_regs(void *km, int n_regs, mm_reg1_t *regs);
int mm_squeeze_a(void *km, int n_regs, mm_reg1_t *regs, mm128_t *a);
int mm_set_sam_pri(int n, mm_reg1_t *r);
void mm_set_parent(void *km, float mask_level, int mask_len, int n, mm_reg1_t *r, int sub_diff, int hard_mask_level, float alt_diff_frac);
void mm_select_sub(void *km, float pri_ratio, float pri_ratio_max, int min_diff, int best_n, int best_n_max, int *n_, mm_reg1_t *r);
void mm_filter_regs(const mm_mapopt_t *opt, int qlen, int *n_regs, mm_reg1_t *regs);
void mm_join_long(void *km, const mm_mapopt_t *opt, int qlen, int *n_regs, mm_reg1_t *regs, mm128_t *a);
void mm_hit_sort(void *km, int *n_regs, mm_reg1_t *r, float alt_diff_frac);
void mm_set_mapq(void *km, int n_regs, mm_reg1_t *regs, int min_chain_sc, int match_sc, int rep_len);

void mm_est_err(const mm_idx_t *mi, int qlen, int n_regs, mm_reg1_t *regs, const mm128_t *a, int32_t n, const uint64_t *mini_pos);

mm_seg_t *mm_seg_gen(void *km, uint32_t hash, int n_segs, const int *qlens, int n_regs0, const mm_reg1_t *regs0, int *n_regs, mm_reg1_t **regs, const mm128_t *a);
void mm_seg_free(void *km, int n_segs, mm_seg_t *segs);

FILE *mm_split_init(const char *prefix, const mm_idx_t *mi);
mm_idx_t *mm_split_merge_prep(const char *prefix, int n_splits, FILE **fp, uint32_t *n_seq_part);
int mm_split_merge(int n_segs, const char **fn, const mm_mapopt_t *opt, int n_split_idx);
void mm_split_rm_tmp(const char *prefix, int n_splits);

void mm_err_puts(const char *str);
void mm_err_fwrite(const void *p, size_t size, size_t nitems, FILE *fp);
void mm_err_fread(void *p, size_t size, size_t nitems, FILE *fp);

void *um_didx_gen(const char *fn, int k, int pre, int bf_bits, uint64_t mini_batch_size, int n_thread);
void um_didx_destroy(void *dh);
int um_didx_get(const void *h_, uint64_t x, int skip_bf);
void um_didx_dump(FILE *fp, const void *h_);
void *um_didx_load(FILE *fp);

static inline uint64_t um_hash64(uint64_t key, uint64_t mask)
{
	key = (~key + (key << 21)) & mask; // key = (key << 21) - key - 1;
	key = key ^ key >> 24;
	key = ((key + (key << 3)) + (key << 8)) & mask; // key * 265
	key = key ^ key >> 14;
	key = ((key + (key << 2)) + (key << 4)) & mask; // key * 21
	key = key ^ key >> 28;
	key = (key + (key << 31)) & mask;
	return key;
}

static const char um_LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
	-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
	LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

static inline int um_ilog2_32(uint32_t v)
{
	uint32_t t, tt;
	if ((tt = v>>16)) return (t = tt>>8) ? 24 + um_LogTable256[t] : 16 + um_LogTable256[tt];
	return (t = v>>8) ? 8 + um_LogTable256[t] : um_LogTable256[v];
}

#ifdef __cplusplus
}
#endif

#endif
