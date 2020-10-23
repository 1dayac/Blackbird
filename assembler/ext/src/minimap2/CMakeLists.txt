project(minimap2 C)
cmake_minimum_required(VERSION 3.8)



add_library(minimap2
        kthread.c
        kalloc.c
        ksw2_extz2_sse.c
        ksw2_extd2_sse.c
        ksw2_exts2_sse.c
        misc.c
        bseq.c
        sketch.c
        sdust.c
        options.c
        index.c
        chain.c
        align.c
        hit.c
        map.c
        format.c
        pe.c
        esterr.c
        ksw2_ll_sse.c
        splitidx.c
)
