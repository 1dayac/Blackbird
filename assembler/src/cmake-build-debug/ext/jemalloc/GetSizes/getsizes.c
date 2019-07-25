
    #include <stdlib.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>

    #if defined __APPLE__
        #include <sys/sysctl.h>
    #elif defined __linux__
        #include <stdio.h>
    #elif (defined(__FreeBSD__) || defined(__NetBSD__))
        #include <sys/param.h>
    #else
        #error unrecognized platform
    #endif
#endif

#include <stdio.h>
int main() {
    long result = 0;
    size_t cache_line_size = 0;
    #ifdef _WIN32
        SYSTEM_INFO si;
        DWORD bufferSize = 0;
        DWORD i = 0;
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer = 0;

        GetSystemInfo(&si);
        result = si.dwSizes;
    #else
        result = sysconf(_SC_PAGESIZE);
    #endif
    printf("%ld;", result);

    #if defined _WIN32
        GetLogicalProcessorInformation(0, &bufferSize);
        buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) malloc(bufferSize);
        GetLogicalProcessorInformation(&buffer[0], &bufferSize);

        for (i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
            if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
                cache_line_size = buffer[i].Cache.LineSize;
                break;
            }
        }
        free(buffer);
    #elif defined __APPLE__
        size_t sizeof_line_size = sizeof(cache_line_size);
        if (sysctlbyname("hw.cachelinesize", &cache_line_size, &sizeof_line_size, 0, 0)) {
            abort();
        }
    #elif defined __linux__
        #if defined(_SC_LEVEL1_DCACHE_LINESIZE)
            cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        #else
            FILE *p = NULL;
            p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
            if (p) {
                if (1 != fscanf(p, "%zu", &cache_line_size)) {
                    abort();
                }
                fclose(p);
            } else {
                abort();
            }
        #endif
    #elif (defined(__FreeBSD__) || defined(__NetBSD__))
        cache_line_size = CACHE_LINE_SIZE;
    #endif
    printf("%ld", cache_line_size);

    return 0;
}
