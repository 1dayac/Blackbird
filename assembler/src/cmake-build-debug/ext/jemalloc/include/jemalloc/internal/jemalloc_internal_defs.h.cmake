#ifndef JEMALLOC_INTERNAL_DEFS_H_
#define JEMALLOC_INTERNAL_DEFS_H_
/*
 * If JEMALLOC_PREFIX is defined via --with-jemalloc-prefix, it will cause all
 * public APIs to be prefixed.  This makes it possible, with some care, to use
 * multiple allocators simultaneously.
 */
#cmakedefine JEMALLOC_PREFIX @JEMALLOC_PREFIX@
#cmakedefine JEMALLOC_CPREFIX @JEMALLOC_CPREFIX@

/*
 * Define overrides for non-standard allocator-related functions if they are
 * present on the system.
 */
#cmakedefine JEMALLOC_OVERRIDE___LIBC_CALLOC @JEMALLOC_OVERRIDE___LIBC_CALLOC@
#cmakedefine JEMALLOC_OVERRIDE___LIBC_FREE @JEMALLOC_OVERRIDE___LIBC_FREE@
#cmakedefine JEMALLOC_OVERRIDE___LIBC_MALLOC @JEMALLOC_OVERRIDE___LIBC_MALLOC@
#cmakedefine JEMALLOC_OVERRIDE___LIBC_MEMALIGN @JEMALLOC_OVERRIDE___LIBC_MEMALIGN@
#cmakedefine JEMALLOC_OVERRIDE___LIBC_REALLOC @JEMALLOC_OVERRIDE___LIBC_REALLOC@
#cmakedefine JEMALLOC_OVERRIDE___LIBC_VALLOC @JEMALLOC_OVERRIDE___LIBC_VALLOC@
#cmakedefine JEMALLOC_OVERRIDE___POSIX_MEMALIGN @JEMALLOC_OVERRIDE___POSIX_MEMALIGN@

/*
 * JEMALLOC_PRIVATE_NAMESPACE is used as a prefix for all library-private APIs.
 * For shared libraries, symbol visibility mechanisms prevent these symbols
 * from being exported, but for static libraries, naming collisions are a real
 * possibility.
 */
#cmakedefine JEMALLOC_PRIVATE_NAMESPACE @JEMALLOC_PRIVATE_NAMESPACE@

/*
 * Hyper-threaded CPUs may need a special instruction inside spin loops in
 * order to yield to another virtual CPU.
 */
#cmakedefine CPU_SPINWAIT @CPU_SPINWAIT@
/* 1 if CPU_SPINWAIT is defined, 0 otherwise. */
#cmakedefine HAVE_CPU_SPINWAIT @HAVE_CPU_SPINWAIT@

/*
 * Number of significant bits in virtual addresses.  This may be less than the
 * total number of bits in a pointer, e.g. on x64, for which the uppermost 16
 * bits are the same as bit 47.
 */
#cmakedefine LG_VADDR @LG_VADDR@

/* Defined if C11 atomics are available. */
#cmakedefine JEMALLOC_C11_ATOMICS @JEMALLOC_C11_ATOMICS@

/* Defined if GCC __atomic atomics are available. */
#cmakedefine JEMALLOC_GCC_ATOMIC_ATOMICS @JEMALLOC_GCC_ATOMIC_ATOMICS@
/* and the 8-bit variant support. */
#cmakedefine JEMALLOC_GCC_U8_ATOMIC_ATOMICS @JEMALLOC_GCC_U8_ATOMIC_ATOMICS@

/* Defined if GCC __sync atomics are available. */
#cmakedefine JEMALLOC_GCC_SYNC_ATOMICS @JEMALLOC_GCC_SYNC_ATOMICS@
/* and the 8-bit variant support. */
#cmakedefine JEMALLOC_GCC_U8_SYNC_ATOMICS @JEMALLOC_GCC_U8_SYNC_ATOMICS@

/*
 * Defined if __builtin_clz() and __builtin_clzl() are available.
 */
#cmakedefine JEMALLOC_HAVE_BUILTIN_CLZ @JEMALLOC_HAVE_BUILTIN_CLZ@

/*
 * Defined if os_unfair_lock_*() functions are available, as provided by Darwin.
 */
#cmakedefine JEMALLOC_OS_UNFAIR_LOCK @JEMALLOC_OS_UNFAIR_LOCK@

/* Defined if syscall(2) is usable. */
#cmakedefine JEMALLOC_USE_SYSCALL @JEMALLOC_USE_SYSCALL@

/*
 * Defined if secure_getenv(3) is available.
 */
#cmakedefine JEMALLOC_HAVE_SECURE_GETENV @JEMALLOC_HAVE_SECURE_GETENV@

/*
 * Defined if issetugid(2) is available.
 */
#cmakedefine JEMALLOC_HAVE_ISSETUGID @JEMALLOC_HAVE_ISSETUGID@

/* Defined if pthread_atfork(3) is available. */
#cmakedefine JEMALLOC_HAVE_PTHREAD_ATFORK @JEMALLOC_HAVE_PTHREAD_ATFORK@

/* Defined if pthread_setname_np(3) is available. */
#cmakedefine JEMALLOC_HAVE_PTHREAD_SETNAME_NP @JEMALLOC_HAVE_PTHREAD_SETNAME_NP@

/*
 * Defined if clock_gettime(CLOCK_MONOTONIC_COARSE, ...) is available.
 */
#cmakedefine JEMALLOC_HAVE_CLOCK_MONOTONIC_COARSE @JEMALLOC_HAVE_CLOCK_MONOTONIC_COARSE@

/*
 * Defined if clock_gettime(CLOCK_MONOTONIC, ...) is available.
 */
#cmakedefine JEMALLOC_HAVE_CLOCK_MONOTONIC @JEMALLOC_HAVE_CLOCK_MONOTONIC@

/*
 * Defined if mach_absolute_time() is available.
 */
#cmakedefine JEMALLOC_HAVE_MACH_ABSOLUTE_TIME @JEMALLOC_HAVE_MACH_ABSOLUTE_TIME@

/*
 * Defined if _malloc_thread_cleanup() exists.  At least in the case of
 * FreeBSD, pthread_key_create() allocates, which if used during malloc
 * bootstrapping will cause recursion into the pthreads library.  Therefore, if
 * _malloc_thread_cleanup() exists, use it as the basis for thread cleanup in
 * malloc_tsd.
 */
#cmakedefine JEMALLOC_MALLOC_THREAD_CLEANUP @JEMALLOC_MALLOC_THREAD_CLEANUP@

/*
 * Defined if threaded initialization is known to be safe on this platform.
 * Among other things, it must be possible to initialize a mutex without
 * triggering allocation in order for threaded allocation to be safe.
 */
#cmakedefine JEMALLOC_THREADED_INIT @JEMALLOC_THREADED_INIT@

/*
 * Defined if the pthreads implementation defines
 * _pthread_mutex_init_calloc_cb(), in which case the function is used in order
 * to avoid recursive allocation during mutex initialization.
 */
#cmakedefine JEMALLOC_MUTEX_INIT_CB @JEMALLOC_MUTEX_INIT_CB@

/* Non-empty if the tls_model attribute is supported. */
#cmakedefine JEMALLOC_TLS_MODEL @JEMALLOC_TLS_MODEL@

/*
 * JEMALLOC_DEBUG enables assertions and other sanity checks, and disables
 * inline functions.
 */
#cmakedefine JEMALLOC_DEBUG @JEMALLOC_DEBUG@

/* JEMALLOC_STATS enables statistics calculation. */
#cmakedefine JEMALLOC_STATS @JEMALLOC_STATS@

/* JEMALLOC_EXPERIMENTAL_SMALLOCX_API enables experimental smallocx API. */
#cmakedefine JEMALLOC_EXPERIMENTAL_SMALLOCX_API @JEMALLOC_EXPERIMENTAL_SMALLOCX_API@

/* JEMALLOC_PROF enables allocation profiling. */
#cmakedefine JEMALLOC_PROF @JEMALLOC_PROF@

/* Use libunwind for profile backtracing if defined. */
#cmakedefine JEMALLOC_PROF_LIBUNWIND @JEMALLOC_PROF_LIBUNWIND@

/* Use libgcc for profile backtracing if defined. */
#cmakedefine JEMALLOC_PROF_LIBGCC @JEMALLOC_PROF_LIBGCC@

/* Use gcc intrinsics for profile backtracing if defined. */
#cmakedefine JEMALLOC_PROF_GCC @JEMALLOC_PROF_GCC@

/*
 * JEMALLOC_DSS enables use of sbrk(2) to allocate extents from the data storage
 * segment (DSS).
 */
#cmakedefine JEMALLOC_DSS @JEMALLOC_DSS@

/* Support memory filling (junk/zero). */
#cmakedefine JEMALLOC_FILL @JEMALLOC_FILL@

/* Support utrace(2)-based tracing. */
#cmakedefine JEMALLOC_UTRACE @JEMALLOC_UTRACE@

/* Support optional abort() on OOM. */
#cmakedefine JEMALLOC_XMALLOC @JEMALLOC_XMALLOC@

/* Support lazy locking (avoid locking unless a second thread is launched). */
#cmakedefine JEMALLOC_LAZY_LOCK @JEMALLOC_LAZY_LOCK@

/*
 * Minimum allocation alignment is 2^LG_QUANTUM bytes (ignoring tiny size
 * classes).
 */
#cmakedefine LG_QUANTUM @LG_QUANTUM@

/* One page is 2^LG_PAGE bytes. */
#cmakedefine LG_PAGE @LG_PAGE@

/*
 * One huge page is 2^LG_HUGEPAGE bytes.  Note that this is defined even if the
 * system does not explicitly support huge pages; system calls that require
 * explicit huge page support are separately configured.
 */
#cmakedefine LG_HUGEPAGE @LG_HUGEPAGE@

/*
 * If defined, adjacent virtual memory mappings with identical attributes
 * automatically coalesce, and they fragment when changes are made to subranges.
 * This is the normal order of things for mmap()/munmap(), but on Windows
 * VirtualAlloc()/VirtualFree() operations must be precisely matched, i.e.
 * mappings do *not* coalesce/fragment.
 */
#cmakedefine JEMALLOC_MAPS_COALESCE @JEMALLOC_MAPS_COALESCE@

/*
 * If defined, retain memory for later reuse by default rather than using e.g.
 * munmap() to unmap freed extents.  This is enabled on 64-bit Linux because
 * common sequences of mmap()/munmap() calls will cause virtual memory map
 * holes.
 */
#cmakedefine JEMALLOC_RETAIN @JEMALLOC_RETAIN@

/* TLS is used to map arenas and magazine caches to threads. */
#cmakedefine JEMALLOC_TLS @JEMALLOC_TLS@

/*
 * Used to mark unreachable code to quiet "end of non-void" compiler warnings.
 * Don't use this directly; instead use unreachable() from util.h
 */
#cmakedefine JEMALLOC_INTERNAL_UNREACHABLE @JEMALLOC_INTERNAL_UNREACHABLE@

/*
 * ffs*() functions to use for bitmapping.  Don't use these directly; instead,
 * use ffs_*() from util.h.
 */
#cmakedefine JEMALLOC_INTERNAL_FFSLL @JEMALLOC_INTERNAL_FFSLL@
#cmakedefine JEMALLOC_INTERNAL_FFSL @JEMALLOC_INTERNAL_FFSL@
#cmakedefine JEMALLOC_INTERNAL_FFS @JEMALLOC_INTERNAL_FFS@

/*
 * popcount*() functions to use for bitmapping.
 */
#cmakedefine JEMALLOC_INTERNAL_POPCOUNTL @JEMALLOC_INTERNAL_POPCOUNTL@
#cmakedefine JEMALLOC_INTERNAL_POPCOUNT @JEMALLOC_INTERNAL_POPCOUNT@

/*
 * If defined, explicitly attempt to more uniformly distribute large allocation
 * pointer alignments across all cache indices.
 */
#cmakedefine JEMALLOC_CACHE_OBLIVIOUS @JEMALLOC_CACHE_OBLIVIOUS@

/*
 * If defined, enable logging facilities.  We make this a configure option to
 * avoid taking extra branches everywhere.
 */
#cmakedefine JEMALLOC_LOG @JEMALLOC_LOG@

/*
 * If defined, use readlinkat() (instead of readlink()) to follow
 * /etc/malloc_conf.
 */
#cmakedefine JEMALLOC_READLINKAT @JEMALLOC_READLINKAT@

/*
 * Darwin (OS X) uses zones to work around Mach-O symbol override shortcomings.
 */
#cmakedefine JEMALLOC_ZONE @JEMALLOC_ZONE@

/*
 * Methods for determining whether the OS overcommits.
 * JEMALLOC_PROC_SYS_VM_OVERCOMMIT_MEMORY: Linux's
 *                                         /proc/sys/vm.overcommit_memory file.
 * JEMALLOC_SYSCTL_VM_OVERCOMMIT: FreeBSD's vm.overcommit sysctl.
 */
#cmakedefine JEMALLOC_SYSCTL_VM_OVERCOMMIT @JEMALLOC_SYSCTL_VM_OVERCOMMIT@
#cmakedefine JEMALLOC_PROC_SYS_VM_OVERCOMMIT_MEMORY @JEMALLOC_PROC_SYS_VM_OVERCOMMIT_MEMORY@

/* Defined if madvise(2) is available. */
#cmakedefine JEMALLOC_HAVE_MADVISE @JEMALLOC_HAVE_MADVISE@

/*
 * Defined if transparent huge pages are supported via the MADV_[NO]HUGEPAGE
 * arguments to madvise(2).
 */
#cmakedefine JEMALLOC_HAVE_MADVISE_HUGE @JEMALLOC_HAVE_MADVISE_HUGE@

/*
 * Methods for purging unused pages differ between operating systems.
 *
 *   madvise(..., MADV_FREE) : This marks pages as being unused, such that they
 *                             will be discarded rather than swapped out.
 *   madvise(..., MADV_DONTNEED) : If JEMALLOC_PURGE_MADVISE_DONTNEED_ZEROS is
 *                                 defined, this immediately discards pages,
 *                                 such that new pages will be demand-zeroed if
 *                                 the address region is later touched;
 *                                 otherwise this behaves similarly to
 *                                 MADV_FREE, though typically with higher
 *                                 system overhead.
 */
#cmakedefine JEMALLOC_PURGE_MADVISE_FREE @JEMALLOC_PURGE_MADVISE_FREE@
#cmakedefine JEMALLOC_PURGE_MADVISE_DONTNEED @JEMALLOC_PURGE_MADVISE_DONTNEED@
#cmakedefine JEMALLOC_PURGE_MADVISE_DONTNEED_ZEROS @JEMALLOC_PURGE_MADVISE_DONTNEED_ZEROS@

/* Defined if madvise(2) is available but MADV_FREE is not (x86 Linux only). */
#cmakedefine JEMALLOC_DEFINE_MADVISE_FREE @JEMALLOC_DEFINE_MADVISE_FREE@

/*
 * Defined if MADV_DO[NT]DUMP is supported as an argument to madvise.
 */
#cmakedefine JEMALLOC_MADVISE_DONTDUMP @JEMALLOC_MADVISE_DONTDUMP@

/*
 * Defined if transparent huge pages (THPs) are supported via the
 * MADV_[NO]HUGEPAGE arguments to madvise(2), and THP support is enabled.
 */
#cmakedefine JEMALLOC_THP @JEMALLOC_THP@

/* Define if operating system has alloca.h header. */
#cmakedefine JEMALLOC_HAS_ALLOCA_H @JEMALLOC_HAS_ALLOCA_H@

/* C99 restrict keyword supported. */
#cmakedefine JEMALLOC_HAS_RESTRICT @JEMALLOC_HAS_RESTRICT@

/* For use by hash code. */
#cmakedefine JEMALLOC_BIG_ENDIAN @JEMALLOC_BIG_ENDIAN@

/* sizeof(int) == 2^LG_SIZEOF_INT. */
#cmakedefine LG_SIZEOF_INT @LG_SIZEOF_INT@

/* sizeof(long) == 2^LG_SIZEOF_LONG. */
#cmakedefine LG_SIZEOF_LONG @LG_SIZEOF_LONG@

/* sizeof(long long) == 2^LG_SIZEOF_LONG_LONG. */
#cmakedefine LG_SIZEOF_LONG_LONG @LG_SIZEOF_LONG_LONG@

/* sizeof(intmax_t) == 2^LG_SIZEOF_INTMAX_T. */
#cmakedefine LG_SIZEOF_INTMAX_T @LG_SIZEOF_INTMAX_T@

/* glibc malloc hooks (__malloc_hook, __realloc_hook, __free_hook). */
#cmakedefine JEMALLOC_GLIBC_MALLOC_HOOK @JEMALLOC_GLIBC_MALLOC_HOOK@

/* glibc memalign hook. */
#cmakedefine JEMALLOC_GLIBC_MEMALIGN_HOOK @JEMALLOC_GLIBC_MEMALIGN_HOOK@

/* pthread support */
#cmakedefine JEMALLOC_HAVE_PTHREAD @JEMALLOC_HAVE_PTHREAD@

/* dlsym() support */
#cmakedefine JEMALLOC_HAVE_DLSYM @JEMALLOC_HAVE_DLSYM@

/* Adaptive mutex support in pthreads. */
#cmakedefine JEMALLOC_HAVE_PTHREAD_MUTEX_ADAPTIVE_NP @JEMALLOC_HAVE_PTHREAD_MUTEX_ADAPTIVE_NP@

/* GNU specific sched_getcpu support */
#cmakedefine JEMALLOC_HAVE_SCHED_GETCPU @JEMALLOC_HAVE_SCHED_GETCPU@

/* GNU specific sched_setaffinity support */
#cmakedefine JEMALLOC_HAVE_SCHED_SETAFFINITY @JEMALLOC_HAVE_SCHED_SETAFFINITY@

/*
 * If defined, all the features necessary for background threads are present.
 */
#cmakedefine JEMALLOC_BACKGROUND_THREAD @JEMALLOC_BACKGROUND_THREAD@

/*
 * If defined, jemalloc symbols are not exported (doesn't work when
 * JEMALLOC_PREFIX is not defined).
 */
#cmakedefine JEMALLOC_EXPORT @JEMALLOC_EXPORT@

/* config.malloc_conf options string. */
#cmakedefine JEMALLOC_CONFIG_MALLOC_CONF @JEMALLOC_CONFIG_MALLOC_CONF@

/* If defined, jemalloc takes the malloc/free/etc. symbol names. */
#cmakedefine JEMALLOC_IS_MALLOC @JEMALLOC_IS_MALLOC@

/*
 * Defined if strerror_r returns char * if _GNU_SOURCE is defined.
 */
#cmakedefine JEMALLOC_STRERROR_R_RETURNS_CHAR_WITH_GNU_SOURCE @JEMALLOC_STRERROR_R_RETURNS_CHAR_WITH_GNU_SOURCE@

/* Performs additional size-matching sanity checks when defined. */
#cmakedefine JEMALLOC_EXTRA_SIZE_CHECK @JEMALLOC_EXTRA_SIZE_CHECK@

#endif /* JEMALLOC_INTERNAL_DEFS_H_ */
