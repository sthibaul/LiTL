/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "litl_merge.h"

static evnt_trace_merge_t* __archive;

/*
 * Create and open an archive for traces
 * Allocate memory for the buffer
 */
static void __init_trace(char *trace_name) {
    __archive = malloc(sizeof(evnt_trace_merge_t));

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((__archive->f_arch = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", trace_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    __archive->buffer_size = 16 * 1024 * 1024; // 16 MB
    __archive->buffer = (evnt_buffer_t) malloc(__archive->buffer_size);
    __archive->header_offset = 0;
    __archive->general_offset = 0;
}

/*
 * This function adds a trace header:
 *   - The number of traces
 *   - Triples: a file id, a file size, and an offset
 */
static void __add_trace_header(evnt_size_t nb_traces) {
    evnt_size_t header_size;

    // add nb_traces and the is_trace_archive flag
    ((evnt_header_t *) __archive->buffer)->nb_threads = nb_traces;
    ((evnt_header_t *) __archive->buffer)->is_trace_archive = 1;
    header_size = sizeof(evnt_size_t) + sizeof(evnt_tiny_size_t);
    __archive->header_offset += header_size;
    write(__archive->f_arch, __archive->buffer, header_size);

    // we do not add all the information about each trace 'cause it will be added during packing them,
    //       instead we just reserve the space for that
    header_size += nb_traces * sizeof(evnt_header_triples_t);
    __archive->general_offset += header_size;
    lseek(__archive->f_arch, header_size, SEEK_SET);
}

/*
 * This function for merging trace files is a modified version of the cat implementation from the Kernighan & Ritchie book
 */
void litl_merge_file(const int file_id, const char *file_name_in) {
    int trace_in, res;
    evnt_trace_size_t file_size;

    if ((trace_in = open(file_name_in, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", file_name_in);
        exit(EXIT_FAILURE);
    }

    // find the trace size
    struct stat st;
    if (fstat(trace_in, &st)) {
        perror("Cannot apply fstat to the input trace files!");
        exit(EXIT_FAILURE);
    }
    file_size = st.st_size;

    // add triples (fid, file_size, offset)
    lseek(__archive->f_arch, __archive->header_offset, SEEK_SET);
    write(__archive->f_arch, &file_id, sizeof(evnt_tid_t));
    write(__archive->f_arch, &file_size, sizeof(evnt_trace_size_t));
    write(__archive->f_arch, &__archive->general_offset, sizeof(evnt_offset_t));
    lseek(__archive->f_arch, __archive->general_offset, SEEK_SET);
    __archive->header_offset += sizeof(evnt_header_triples_t);

    // solution A: Reading and writing blocks of data. Use the file size to deal with the reading of the last block from
    //             the traces
    while ((res = read(trace_in, __archive->buffer, __archive->buffer_size)) != 0) {
        if (res < 0) {
            perror("Cannot read the data from the traces!");
            exit(EXIT_FAILURE);
        }

        write(__archive->f_arch, __archive->buffer, res);
        __archive->general_offset += res;
    }

    /*// Solution B: Reading and writing characters
     int c;
     while ((c = getc(trace_in)) != EOF)
     putc(c, __archive->f_arch);*/

    close(trace_in);
}

static void __finalize_trace() {
    free(__archive->buffer);
    close(__archive->f_arch);
    free(__archive);
}

int main(int argc, const char **argv) {
    char *prog, *file_name_out;

    // TODO: check for the arguments
    prog = argv[0];
    file_name_out = argv[argc - 1];
    /*if (strcmp(argv[argc - 2], ">")) {
     perror("Specify an archive trace file leading by the '>' character!");
     exit(EXIT_FAILURE);
     }*/

    // init a buffer and an archive of traces
    __init_trace(file_name_out);

    // write header with #traces and reserved space for pairs (fid, offset)
    __add_trace_header(argc - 3);

    // merging the trace files
    // TODO: 2 needs to be changed when the "-o trace_name_out" is right after "litl_merge"
    // TODO: use more meaningful file_id
    while (--argc > 2)
        litl_merge_file(argc, *++argv);

    /*// error handling: valid ONLY for FILE*
     if (ferror(trace_out)) {
     fprintf(stderr, "%s: error while merging trace files\n", prog);
     exit(EXIT_FAILURE);
     }*/

    // finalizing merging
    __finalize_trace();

    return EXIT_SUCCESS;
}
