/* Defined if __attribute__((...)) syntax is supported. */
#cmakedefine JEMALLOC_HAVE_ATTR @JEMALLOC_HAVE_ATTR@

/* Defined if alloc_size attribute is supported. */
#cmakedefine JEMALLOC_HAVE_ATTR_ALLOC_SIZE @JEMALLOC_HAVE_ATTR_ALLOC_SIZE@

/* Defined if format(gnu_printf, ...) attribute is supported. */
#cmakedefine JEMALLOC_HAVE_ATTR_FORMAT_GNU_PRINTF @JEMALLOC_HAVE_ATTR_FORMAT_GNU_PRINTF@

/* Defined if format(printf, ...) attribute is supported. */
#cmakedefine JEMALLOC_HAVE_ATTR_FORMAT_PRINTF @JEMALLOC_HAVE_ATTR_FORMAT_PRINTF@

/*
 * Define overrides for non-standard allocator-related functions if they are
 * present on the system.
 */
#cmakedefine JEMALLOC_OVERRIDE_MEMALIGN @JEMALLOC_OVERRIDE_MEMALIGN@
#cmakedefine JEMALLOC_OVERRIDE_VALLOC @JEMALLOC_OVERRIDE_VALLOC@

/*
 * At least Linux omits the "const" in:
 *
 *   size_t malloc_usable_size(const void *ptr);
 *
 * Match the operating system's prototype.
 */
#cmakedefine JEMALLOC_USABLE_SIZE_CONST @JEMALLOC_USABLE_SIZE_CONST@

/*
 * If defined, specify throw() for the public function prototypes when compiling
 * with C++.  The only justification for this is to match the prototypes that
 * glibc defines.
 */
#cmakedefine JEMALLOC_USE_CXX_THROW @JEMALLOC_USE_CXX_THROW@

#ifdef _MSC_VER
#  ifdef _WIN64
#    define LG_SIZEOF_PTR_WIN 3
#  else
#    define LG_SIZEOF_PTR_WIN 2
#  endif
#endif

/* sizeof(void *) == 2^LG_SIZEOF_PTR. */
#cmakedefine LG_SIZEOF_PTR @LG_SIZEOF_PTR@
