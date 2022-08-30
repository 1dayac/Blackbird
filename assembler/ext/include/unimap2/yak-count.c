#include "umpriv.h"

/*********************
 * Mostly from yak.h *
 *********************/

#include <stdint.h>
#include "khashl.h"

#define YAK_MAX_KMER     31
#define YAK_COUNTER_BITS 12
#define YAK_N_COUNTS     (1<<YAK_COUNTER_BITS)
#define YAK_MAX_COUNT    ((1<<YAK_COUNTER_BITS)-1)

#define YAK_BLK_SHIFT  9 // 64 bytes, the size of a cache line
#define YAK_BLK_MASK   ((1<<(YAK_BLK_SHIFT)) - 1)

#define yak_ch_eq(a, b) ((a)>>YAK_COUNTER_BITS == (b)>>YAK_COUNTER_BITS) // lower 8 bits for counts; higher bits for k-mer
#define yak_ch_hash(a) ((a)>>YAK_COUNTER_BITS)
KHASHL_SET_INIT(, yak_ht_t, yak_ht, uint64_t, yak_ch_hash, yak_ch_eq)

typedef struct {
	int32_t bf_shift, bf_n_hash;
	int32_t k;
	int32_t pre;
	int32_t n_thread;
	int64_t chunk_size;
} yak_copt_t;

typedef struct {
	int n_shift, n_hashes;
	uint8_t *b;
} yak_bf_t;

typedef struct {
	yak_ht_t *h;
	yak_bf_t *b;
} yak_ch1_t;

typedef struct {
	int k, pre, n_hash, n_shift;
	uint64_t tot;
	yak_ch1_t *h;
} yak_ch_t;

#define YCALLOC(ptr, len) ((ptr) = (__typeof__(ptr))calloc((len), sizeof(*(ptr))))
#define YMALLOC(ptr, len) ((ptr) = (__typeof__(ptr))malloc((len) * sizeof(*(ptr))))
#define YREALLOC(ptr, len) ((ptr) = (__typeof__(ptr))realloc((ptr), (len) * sizeof(*(ptr))))

/*************************
 * From bbf.c and htab.c *
 *************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "kthread.h"

/*** Blocked bloom filter ***/

yak_bf_t *yak_bf_init(int n_shift, int n_hashes)
{
	yak_bf_t *b;
	void *ptr = 0;
	if (n_shift + YAK_BLK_SHIFT > 64 || n_shift < YAK_BLK_SHIFT) return 0;
	YCALLOC(b, 1);
	b->n_shift = n_shift;
	b->n_hashes = n_hashes;
	posix_memalign(&ptr, 1<<(YAK_BLK_SHIFT-3), 1ULL<<(n_shift-3));
	b->b = (uint8_t*)ptr;
	bzero(b->b, 1ULL<<(n_shift-3));
	return b;
}

void yak_bf_destroy(yak_bf_t *b)
{
	if (b == 0) return;
	free(b->b); free(b);
}

int yak_bf_insert(yak_bf_t *b, uint64_t hash)
{
	int x = b->n_shift - YAK_BLK_SHIFT;
	uint64_t y = hash & ((1ULL<<x) - 1);
	int h1 = hash >> x & YAK_BLK_MASK;
	int h2 = hash >> b->n_shift & YAK_BLK_MASK;
	uint8_t *p = &b->b[y<<(YAK_BLK_SHIFT-3)];
	int i, z = h1, cnt = 0;
	if ((h2&31) == 0) h2 = (h2 + 1) & YAK_BLK_MASK; // otherwise we may repeatedly use a few bits
	for (i = 0; i < b->n_hashes; z = (z + h2) & YAK_BLK_MASK) {
		uint8_t *q = &p[z>>3], u;
		u = 1<<(z&7);
		cnt += !!(*q & u);
		*q |= u;
		++i;
	}
	return cnt;
}

int yak_bf_get(const yak_bf_t *b, uint64_t hash)
{
	int x = b->n_shift - YAK_BLK_SHIFT;
	uint64_t y = hash & ((1ULL<<x) - 1);
	int h1 = hash >> x & YAK_BLK_MASK;
	int h2 = hash >> b->n_shift & YAK_BLK_MASK;
	const uint8_t *p = &b->b[y<<(YAK_BLK_SHIFT-3)];
	int i, z = h1, cnt = 0;
	if ((h2&31) == 0) h2 = (h2 + 1) & YAK_BLK_MASK; // otherwise we may repeatedly use a few bits
	for (i = 0; i < b->n_hashes; z = (z + h2) & YAK_BLK_MASK) {
		const uint8_t *q = &p[z>>3];
		uint8_t u;
		u = 1<<(z&7);
		cnt += !!(*q & u);
		++i;
	}
	return cnt;
}

/*** hash table ***/

yak_ch_t *yak_ch_init(int k, int pre, int n_hash, int n_shift)
{
	yak_ch_t *h;
	int i;
	if (pre < YAK_COUNTER_BITS) return 0;
	YCALLOC(h, 1);
	h->k = k, h->pre = pre;
	YCALLOC(h->h, 1<<h->pre);
	for (i = 0; i < 1<<h->pre; ++i)
		h->h[i].h = yak_ht_init();
	if (n_hash > 0 && n_shift > h->pre) {
		h->n_hash = n_hash, h->n_shift = n_shift;
		for (i = 0; i < 1<<h->pre; ++i)
			h->h[i].b = yak_bf_init(h->n_shift - h->pre, h->n_hash);
	}
	return h;
}

void yak_ch_destroy_bf(yak_ch_t *h)
{
	int i;
	for (i = 0; i < 1<<h->pre; ++i) {
		if (h->h[i].b)
			yak_bf_destroy(h->h[i].b);
		h->h[i].b = 0;
	}
}

void yak_ch_destroy(yak_ch_t *h)
{
	int i;
	if (h == 0) return;
	yak_ch_destroy_bf(h);
	for (i = 0; i < 1<<h->pre; ++i)
		yak_ht_destroy(h->h[i].h);
	free(h->h); free(h);
}

int yak_ch_get(const yak_ch_t *h, uint64_t x)
{
	int mask = (1<<h->pre) - 1;
	yak_ht_t *g = h->h[x&mask].h;
	khint_t k;
	k = yak_ht_get(g, x >> h->pre << YAK_COUNTER_BITS);
	return k == kh_end(g)? -1 : kh_key(g, k)&YAK_MAX_COUNT;
}

int yak_ch_insert_list(yak_ch_t *h, int create_new, int n, const uint64_t *a)
{
	int j, mask = (1<<h->pre) - 1, n_ins = 0;
	yak_ch1_t *g;
	if (n == 0) return 0;
	g = &h->h[a[0]&mask];
	for (j = 0; j < n; ++j) {
		int ins = 1, absent;
		uint64_t x = a[j] >> h->pre;
		khint_t k;
		if ((a[j]&mask) != (a[0]&mask)) continue;
		if (create_new) {
			if (g->b)
				ins = (yak_bf_insert(g->b, x) == h->n_hash);
			if (ins) {
				k = yak_ht_put(g->h, x<<YAK_COUNTER_BITS, &absent);
				if (absent) ++n_ins;
				if ((kh_key(g->h, k)&YAK_MAX_COUNT) < YAK_MAX_COUNT)
					++kh_key(g->h, k);
			}
		} else {
			k = yak_ht_get(g->h, x<<YAK_COUNTER_BITS);
			if (k != kh_end(g->h) && (kh_key(g->h, k)&YAK_MAX_COUNT) < YAK_MAX_COUNT)
				++kh_key(g->h, k);
		}
	}
	return n_ins;
}

/*** Clear all counts to 0 ***/

static void worker_clear(void *data, long i, int tid) // callback for kt_for()
{
	yak_ch_t *h = (yak_ch_t*)data;
	yak_ht_t *g = h->h[i].h;
	khint_t k;
	uint64_t mask = ~1ULL >> YAK_COUNTER_BITS << YAK_COUNTER_BITS;
	for (k = 0; k < kh_end(g); ++k)
		if (kh_exist(g, k))
			kh_key(g, k) &= mask;
}

void yak_ch_clear(yak_ch_t *h, int n_thread)
{
	kt_for(n_thread, worker_clear, h, 1<<h->pre);
}

/*** shrink a hash table ***/

typedef struct {
	int min, max;
	yak_ch_t *h;
} shrink_aux_t;

static void worker_shrink(void *data, long i, int tid) // callback for kt_for()
{
	shrink_aux_t *a = (shrink_aux_t*)data;
	yak_ch_t *h = a->h;
	yak_ht_t *g = h->h[i].h, *f;
	khint_t k;
	f = yak_ht_init();
	yak_ht_resize(f, kh_size(g));
	for (k = 0; k < kh_end(g); ++k) {
		if (kh_exist(g, k)) {
			int absent, c = kh_key(g, k) & YAK_MAX_COUNT;
			if (c >= a->min && c <= a->max)
				yak_ht_put(f, kh_key(g, k), &absent);
		}
	}
	yak_ht_destroy(g);
	h->h[i].h = f;
}

void yak_ch_shrink(yak_ch_t *h, int min, int max, int n_thread)
{
	int i;
	shrink_aux_t a;
	a.h = h, a.min = min, a.max = max;
	kt_for(n_thread, worker_shrink, &a, 1<<h->pre);
	for (i = 0, h->tot = 0; i < 1<<h->pre; ++i)
		h->tot += kh_size(h->h[i].h);
}

/****************
 * From count.c *
 ****************/

#include <zlib.h>
#include <string.h>
#include "kseq.h" // FASTA/Q parser
KSEQ_INIT(gzFile, gzread)

void yak_copt_init(yak_copt_t *o)
{
	memset(o, 0, sizeof(yak_copt_t));
	o->bf_shift = 0;
	o->bf_n_hash = 4;
	o->k = 31;
	o->pre = 10;
	o->n_thread = 4;
	o->chunk_size = 10000000;
}

typedef struct {
	int n, m;
	uint64_t n_ins;
	uint64_t *a;
} ch_buf_t;

static inline void ch_insert_buf(ch_buf_t *buf, int p, uint64_t y) // insert a k-mer $y to a linear buffer
{
	int pre = y & ((1<<p) - 1);
	ch_buf_t *b = &buf[pre];
	if (b->n == b->m) {
		b->m = b->m < 8? 8 : b->m + (b->m>>1);
		YREALLOC(b->a, b->m);
	}
	b->a[b->n++] = y;
}

static void count_seq_buf(ch_buf_t *buf, int k, int p, int len, const char *seq) // insert k-mers in $seq to linear buffer $buf
{
	extern unsigned char seq_nt4_table[256];
	int i, l;
	uint64_t x[2], mask = (1ULL<<k*2) - 1, shift = (k - 1) * 2;
	for (i = l = 0, x[0] = x[1] = 0; i < len; ++i) {
		int c = seq_nt4_table[(uint8_t)seq[i]];
		if (c < 4) { // not an "N" base
			x[0] = (x[0] << 2 | c) & mask;                  // forward strand
			x[1] = x[1] >> 2 | (uint64_t)(3 - c) << shift;  // reverse strand
			if (++l >= k) { // we find a k-mer
				uint64_t y = x[0] < x[1]? x[0] : x[1];
				ch_insert_buf(buf, p, um_hash64(y, mask));
			}
		} else l = 0, x[0] = x[1] = 0; // if there is an "N", restart
	}
}

typedef struct { // global data structure for kt_pipeline()
	const yak_copt_t *opt;
	int create_new;
	kseq_t *ks;
	yak_ch_t *h;
} pldat_t;

typedef struct { // data structure for each step in kt_pipeline()
	pldat_t *p;
	int n, m, sum_len, nk;
	int *len;
	char **seq;
	ch_buf_t *buf;
} stepdat_t;

static void worker_for(void *data, long i, int tid) // callback for kt_for()
{
	stepdat_t *s = (stepdat_t*)data;
	ch_buf_t *b = &s->buf[i];
	yak_ch_t *h = s->p->h;
	b->n_ins += yak_ch_insert_list(h, s->p->create_new, b->n, b->a);
}

static void *worker_pipeline(void *data, int step, void *in) // callback for kt_pipeline()
{
	pldat_t *p = (pldat_t*)data;
	if (step == 0) { // step 1: read a block of sequences
		int ret;
		stepdat_t *s;
		YCALLOC(s, 1);
		s->p = p;
		while ((ret = kseq_read(p->ks)) >= 0) {
			int l = p->ks->seq.l;
			if (l < p->opt->k) continue;
			if (s->n == s->m) {
				s->m = s->m < 16? 16 : s->m + (s->n>>1);
				YREALLOC(s->len, s->m);
				YREALLOC(s->seq, s->m);
			}
			YMALLOC(s->seq[s->n], l);
			memcpy(s->seq[s->n], p->ks->seq.s, l);
			s->len[s->n++] = l;
			s->sum_len += l;
			s->nk += l - p->opt->k + 1;
			if (s->sum_len >= p->opt->chunk_size)
				break;
		}
		if (s->sum_len == 0) free(s);
		else return s;
	} else if (step == 1) { // step 2: extract k-mers
		stepdat_t *s = (stepdat_t*)in;
		int i, n = 1<<p->opt->pre, m;
		YCALLOC(s->buf, n);
		m = (int)(s->nk * 1.2 / n) + 1;
		for (i = 0; i < n; ++i) {
			s->buf[i].m = m;
			YMALLOC(s->buf[i].a, m);
		}
		for (i = 0; i < s->n; ++i) {
			count_seq_buf(s->buf, p->opt->k, p->opt->pre, s->len[i], s->seq[i]);
			free(s->seq[i]);
		}
		free(s->seq); free(s->len);
		return s;
	} else if (step == 2) { // step 3: insert k-mers to hash table
		stepdat_t *s = (stepdat_t*)in;
		int i, n = 1<<p->opt->pre;
		uint64_t n_ins = 0;
		kt_for(p->opt->n_thread, worker_for, s, n);
		for (i = 0; i < n; ++i) {
			n_ins += s->buf[i].n_ins;
			free(s->buf[i].a);
		}
		p->h->tot += n_ins;
		free(s->buf);
		//fprintf(stderr, "[M] processed %d sequences; %ld distinct k-mers in the hash table\n", s->n, (long)p->h->tot);
		free(s);
	}
	return 0;
}

yak_ch_t *yak_count(const char *fn, const yak_copt_t *opt, yak_ch_t *h0)
{
	pldat_t pl;
	gzFile fp;
	if ((fp = gzopen(fn, "r")) == 0) return 0;
	pl.ks = kseq_init(fp);
	pl.opt = opt;
	if (h0) {
		pl.h = h0, pl.create_new = 0;
		assert(h0->k == opt->k && h0->pre == opt->pre);
	} else {
		pl.create_new = 1;
		pl.h = yak_ch_init(opt->k, opt->pre, opt->bf_n_hash, opt->bf_shift);
	}
	kt_pipeline(3, worker_pipeline, &pl, 3);
	kseq_destroy(pl.ks);
	gzclose(fp);
	return pl.h;
}

yak_ch_t *yak_count_file(const char *fn1, const char *fn2, const yak_copt_t *opt, int keep_bf)
{
	yak_ch_t *h;
	h = yak_count(fn1, opt, 0); // if bloom filter is in use, this gets approximate counts
	fprintf(stderr, "[M::%s::%.3f*%.2f] round 1: %ld distinct k-mers\n", __func__, realtime() - mm_realtime0, cputime() / (realtime() - mm_realtime0), (long)h->tot);
	if (opt->bf_shift > 0) { // bloom filter is in use
		if (!keep_bf) yak_ch_destroy_bf(h); // deallocate bloom filter
		yak_ch_clear(h, opt->n_thread); // set counts to 0
		h = yak_count(fn2? fn2 : fn1, opt, h); // count again
	}
	yak_ch_shrink(h, 2, YAK_MAX_COUNT, opt->n_thread); // always drop singletons (different from the original yak)
	fprintf(stderr, "[M::%s::%.3f*%.2f] round 2: %ld distinct k-mers\n", __func__, realtime() - mm_realtime0, cputime() / (realtime() - mm_realtime0), (long)h->tot);
	return h;
}

/*******************
 * Main interfaces *
 *******************/

void *um_didx_gen(const char *fn, int k, int pre, int bf_bits, uint64_t mini_batch_size, int n_thread)
{
	yak_ch_t *h;
	yak_copt_t opt;
	if (pre < YAK_COUNTER_BITS) pre = YAK_COUNTER_BITS;
	yak_copt_init(&opt);
	opt.k = k, opt.pre = pre, opt.chunk_size = mini_batch_size, opt.bf_shift = bf_bits;
	h = yak_count_file(fn, fn, &opt, 1);
	return h;
}

void um_didx_destroy(void *h)
{
	yak_ch_destroy((yak_ch_t*)h);
}

int um_didx_get(const void *h_, uint64_t x, int skip_bf)
{
	const yak_ch_t *h = (const yak_ch_t*)h_;
	int c, mask = (1<<h->pre) - 1;
	const yak_ht_t *g = h->h[x&mask].h;
	const yak_bf_t *f = h->h[x&mask].b;
	khint_t k;
	if (!skip_bf && yak_bf_get(f, x >> h->pre) != h->n_hash) return -1;
	k = yak_ht_get(g, x >> h->pre << YAK_COUNTER_BITS);
	if (k == kh_end(g)) return 1; // absent from the hash table; then likely a unique k-mer
	c = kh_key(g, k) & YAK_MAX_COUNT;
	return c == YAK_MAX_COUNT? -2 : c;
}

void um_didx_dump(FILE *fp, const void *h_)
{
	const yak_ch_t *h = (const yak_ch_t*)h_;
	uint32_t t[4], bf_size;
	int i;
	t[0] = h->pre, t[1] = h->k, t[2] = h->n_shift, t[3] = h->n_hash;
	bf_size = 1 << (h->n_shift - h->pre - 3);
	fwrite(t, 4, 4, fp);
	fwrite(&h->tot, 8, 1, fp);
	for (i = 0; i < 1<<h->pre; ++i) {
		yak_ht_t *b = h->h[i].h;
		yak_bf_t *f = h->h[i].b;
		uint32_t size = b? b->count : 0;
		khint_t k;
		fwrite(&size, 4, 1, fp);
		for (k = 0; k < kh_end(b); ++k)
			if (kh_exist(b, k))
				fwrite(&kh_key(b, k), 8, 1, fp);
		if (f->b) fwrite(f->b, 1, bf_size, fp);
	}
}

void *um_didx_load(FILE *fp)
{
	uint32_t t[4], bf_size;
	uint64_t tot;
	int32_t i;
	yak_ch_t *h;
	if (fread(t, 4, 4, fp) != 4) return 0;
	if (fread(&tot, 8, 1, fp) != 1) return 0;
	h = yak_ch_init(t[1], t[0], t[3], t[2]);
	bf_size = 1 << (h->n_shift - h->pre - 3);
	h->tot = tot;
	for (i = 0; i < 1<<h->pre; ++i) {
		uint32_t j, size;
		yak_ht_t *b = h->h[i].h;
		yak_bf_t *f = h->h[i].b;
		fread(&size, 4, 1, fp);
		yak_ht_resize(b, size);
		for (j = 0; j < size; ++j) {
			uint64_t x;
			int absent;
			fread(&x, 8, 1, fp);
			yak_ht_put(b, x, &absent);
			assert(absent);
		}
		fread(f->b, 1, bf_size, fp);
	}
	return h;
}
