#include "jemalloc/internal/jemalloc_internal_defs.h"
#include "jemalloc/internal/jemalloc_internal_decls.h"

/*
 * For use by SFMT.  configure.ac doesn't actually define HAVE_SSE2 because its
 * dependencies are notoriously unportable in practice.
 */
#define HAVE_SSE2 1
/* #undef HAVE_ALTIVEC */
