/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "litl_types.h"
#include "litl_timer.h"
#include "litl_write.h"
#include "litl_read.h"

#define MAX_BUFFER_SIZE 16 * 1024 * 1024
#define NB_EVENTS 1000000

int main(int argc, const char **argv) {
    int i;
    const char* filename = "trace.trace";
    uint32_t buf_size;

    litl_time_t start, fin;
    litl_trace_write_t trace;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_litl_write.trace";

    printf("What is the optimal buffer size for recording events?\n");
    printf("Buffer size[KB] \t Time \n");

    litl_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    buf_size = 1024; // 1KB
    while (buf_size <= MAX_BUFFER_SIZE) {
        trace = litl_init_trace(buf_size);
        litl_set_filename(&trace, filename);

        start = litl_get_time();
        for (i = 0; i < (NB_EVENTS + 1) / 12; i++) {
            litl_probe0(&trace, 0x100 * (i + 1) + 1);
            litl_probe1(&trace, 0x100 * (i + 1) + 2, 1);
            litl_probe2(&trace, 0x100 * (i + 1) + 3, 1, 3);
            litl_probe3(&trace, 0x100 * (i + 1) + 4, 1, 3, 5);
            litl_probe4(&trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
            litl_probe5(&trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
            litl_probe6(&trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
            litl_probe7(&trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13, 17);
            litl_probe8(&trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13, 17, 19);
            litl_probe9(&trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
            litl_probe10(&trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13, 17, 19, 23, 29);
            litl_raw_probe(&trace, 0x100 * (i + 1) + 12, sizeof(val) - 1, val);
        }
        fin = litl_get_time();

        litl_fin_trace(&trace);
        printf("\t%"PRTIu32"\t\t %"PRTIu64"\n", buf_size / 1024, fin - start);

        buf_size = 2 * buf_size;
    }
    printf("NB: time was measured only once on writing %d events to the %s file.\n\n", NB_EVENTS, filename);

    return EXIT_SUCCESS;
}