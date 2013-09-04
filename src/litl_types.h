/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file litl_types.h
 *  \brief litl_types Provides a set of data structures for recording and
 *  reading events as well as merging and splitting trace files
 *
 *  \authors
 *    Developers are: \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#ifndef LITL_TYPES_H_
#define LITL_TYPES_H_

/**
 * \defgroup litl_types LiTL Data Types and Defined Variables
 */

/**
 * \defgroup litl_types_general General Data Types and Defined Variables
 * \ingroup litl_types
 */

/**
 * \defgroup litl_types_write Data Types for Recording Events
 * \ingroup litl_types
 */

/**
 * \defgroup litl_types_read Data Types for Reading Events
 * \ingroup litl_types
 */

/**
 * \defgroup litl_types_merge Data Types for Merging Traces
 * \ingroup litl_types
 */

/**
 * \defgroup litl_types_split Data Types for Splitting Archives of Traces
 * \ingroup litl_types
 */

#include <stdio.h>
#include <stdint.h>

#ifdef USE_GETTID
#include <unistd.h>
#include <sys/syscall.h>  // For SYS_xxx definitions
#else
#include <pthread.h>
#endif

// current thread id
#ifdef USE_GETTID
/**
 * \ingroup litl_types_general
 * \brief A current thread ID
 */
#define CUR_TID syscall(SYS_gettid)
#else
/**
 * \ingroup litl_types_general
 * \brief A current thread ID
 */
#define CUR_TID pthread_self()
#endif

#ifdef __x86_64__
/**
 * \ingroup litl_types_general
 * \brief A data type for storing thread IDs
 */
typedef uint64_t litl_tid_t;
/**
 * \ingroup litl_types_general
 * \brief A data type for storing time stamps
 */
typedef uint64_t litl_time_t;
// TODO: there is a possibility of using uint16_t, however then the alignment
//       would collapse
/**
 * \ingroup litl_types_general
 * \brief A data type for storing events codes
 */
typedef uint32_t litl_code_t;
/**
 * \ingroup litl_types_general
 * \brief A data type for storing traces sizes
 */
typedef uint64_t litl_trace_size_t;
/**
 * \ingroup litl_types_general
 * \brief An auxiliary data type for the optimized storage of data
 */
typedef uint16_t litl_med_size_t;
/**
 * \ingroup litl_types_general
 * \brief An auxiliary data type for storing data
 */
typedef uint32_t litl_size_t;
/**
 * \ingroup litl_types_general
 * \brief A data type for the non-optimized storage of parameters
 */
typedef uint64_t litl_param_t;
/**
 * \ingroup litl_types_general
 * \brief A data type for storing sets of events
 */
typedef uint8_t* litl_buffer_t;
/**
 * \ingroup litl_types_general
 * \brief A data type for storing offsets
 */
typedef uint64_t litl_offset_t;

#elif defined __arm__
typedef uint32_t litl_tid_t;
typedef uint32_t litl_time_t;
typedef uint32_t litl_code_t;
typedef uint32_t litl_trace_size_t;
typedef uint32_t litl_size_t;
typedef uint32_t litl_param_t;
typedef uint8_t* litl_buffer_t;
typedef uint32_t litl_offset_t;
#endif
/**
 * \ingroup litl_types_general
 * \brief A data type for the optimized storage of parameters
 */
typedef uint8_t litl_data_t;

/**
 * \ingroup litl_types_general
 * \brief Defines the code of an event of type offset
 */
#define LITL_OFFSET_CODE 13

/**
 * \ingroup litl_types_general
 * \brief Defines the maximum number of parameters
 */
#define LITL_MAX_PARAMS 10
/**
 * \ingroup litl_types_general
 * \brief Defines the "maximum" size of raw data
 */
#define LITL_MAX_DATA (LITL_MAX_PARAMS * sizeof(litl_param_t))

/**
 * \ingroup litl_types_general
 * \brief The enumeration of event types
 */
typedef enum {
    LITL_TYPE_REGULAR /**< Regular */,
    LITL_TYPE_RAW /**< Raw */,
    LITL_TYPE_PACKED /**< Packed */,
    LITL_TYPE_OFFSET /**< Offset */
}__attribute__((packed)) litl_type_t;

/**
 * \struct litl_t
 * \ingroup litl_types_general
 * \brief A general structure of LiTL event type
 */
typedef struct {
    litl_time_t time; /**< The time of the measurement */
    litl_code_t code; /**< An event code */
    litl_type_t type; /**< An event type */
    /**
     * \union parameters
     * \brief Event parameters
     */
    union {
        /**
         * \struct regular
         * \brief A regular event
         */
        struct {
            litl_data_t nb_params; /**< A number of arguments */
            litl_param_t param[LITL_MAX_PARAMS]; /**< An array of arguments of lengths from 0 to 10 */
        }__attribute__((packed)) regular;
        /**
         * \struct raw
         * \brief A raw event
         */
        struct {
            litl_size_t size; /**< A size of data (in Bytes) */
            litl_data_t data[LITL_MAX_DATA]; /**< A raw data */
        }__attribute__((packed)) raw;
        /**
         * \struct packed
         * \brief A packed event
         */
        struct {
            litl_data_t size; /**< A size of data (in Bytes) */
            litl_data_t param[LITL_MAX_DATA]; /**< A data */
        }__attribute__((packed)) packed;
        /**
         * \struct offset
         * \brief An offset event
         */
        struct {
            litl_data_t nb_params; /**< A number of parameters (=1) */
            litl_param_t offset; /**< An offset to the next chunk of events */
        }__attribute__((packed)) offset;
    } parameters;
}__attribute__((packed)) litl_t;

/**
 * \ingroup litl_types_general
 * \brief Defines the maximum number of threads (pairs of tid and offset) stored in one data
 *  slot
 */
#define NBTHREADS 32

/**
 * \ingroup litl_types_general
 * \brief A general data structure that corresponds to the header of a trace
 *  file
 */
typedef struct {
    litl_med_size_t nb_threads; /**< A total number of threads */
    litl_data_t is_trace_archive; /**< Indicates whether a trace is a regular trace (=0) or an archive of traces (=1) */
    litl_size_t buffer_size; /**< A size of buffer */
    litl_med_size_t header_nb_threads; /**< A number of threads, which info is stored in the header */
    litl_data_t litl_ver[8]; /**< Information regarding the version of LiTL */
    litl_data_t sysinfo[128]; /**< Information regarding OS, Hardware, etc. */
}__attribute__((packed)) litl_header_t;

/**
 * \ingroup litl_types_general
 * \brief A data structure for pairs (tid, offset) stored in the trace header
 */
typedef struct {
    litl_tid_t tid; /**< A thread ID */
    litl_offset_t offset; /**< An offset to the chunk of events */
} litl_header_tids_t;

/**
 * \ingroup litl_types_general
 * \brief A Data structure for triples (tid, trace_size, offset) stored in the
 *  archive's header
 */
typedef struct {
    litl_med_size_t fid; /**< A file ID*/
    litl_trace_size_t trace_size; /**< A trace size */
    litl_offset_t offset; /**< An offset to the merged trace file*/
}__attribute__((packed)) litl_header_triples_t;

/**
 * \ingroup litl_types_write
 * \brief Thread-specific buffer
 */
typedef struct {
    litl_buffer_t buffer_ptr; /**< A pointer to the beginning of the buffer */
    litl_buffer_t buffer_cur; /**< A pointer to the next free slot */
    litl_tid_t tid; /**< An ID of the working thread */
    litl_offset_t offset; /**< An offset to the next buffer in the trace file */

    uint8_t already_flushed; /**< Handles the situation when some threads start after the header was flushed, i.e. their tids and offsets were not included into the header*/
} litl_write_buffer_t;

/**
 * \ingroup litl_types_write
 * \brief A data structure for recording events
 */
typedef struct {
    int f_handle; /**< A file handler */
    char* filename; /**< A file name */

    litl_offset_t general_offset; /**< An offset from the beginning of the trace file to the next free slot */

    litl_buffer_t header_ptr; /**< A pointer to the beginning of the header */
    litl_buffer_t header_cur; /**< A pointer to the next free slot in the header */
    litl_size_t header_size; /**< A header size */
    litl_size_t header_offset; /**< An offset from the beginning of the header to the next free slot */

    litl_med_size_t nb_slots; /**< A number of chunks with the information on threads (tid, offset); first chunk, which is in the header, does not count; each contains at most NBTHREADS threads */
    litl_med_size_t header_nb_threads; /**< A number of threads in the header */
    litl_param_t threads_offset; /**< An offset to the next chunk of pairs (tid, offset) for a given thread */
    uint8_t is_header_flushed; /**< Indicates whether the header with tids and offsets has been flushed*/

    litl_size_t buffer_size; /**< A buffer size */
    uint8_t allow_buffer_flush; /**< Indicates whether buffer flush is enabled (1) or not (0). In case the flushing is disabled, the recording of events is stopped. By default, it is activated */
    uint8_t is_buffer_full; /**< Indicates whether the buffer is full */

    pthread_once_t index_once; /**< Guarantees that the initialization function is called only once */
    pthread_key_t index; /**< A private thread variable that holds its index in tids, buffer_ptr/buffer_cur, and offsets */
    litl_med_size_t nb_threads; /**< A number of threads */

    pthread_mutex_t lock_litl_flush; /**< Handles write conflicts while using pthread */
    pthread_mutex_t lock_buffer_init; /**< Handles race conditions while initializing tids and buffer_ptrs/buffer_curs */
    uint8_t allow_thread_safety; /**< Indicates whether LiTL uses thread-safety (1) or not (0). By default, it is activated */

    uint8_t record_tid_activated; /**< Indicates whether LiTL records tid (1) or not (0). By default, it is activated */
    uint8_t litl_initialized; /**< Ensures that a performance analysis library does not start recording events before the initialization is finished */
    volatile uint8_t litl_paused; /**< Indicates whether LiTL stops recording events (1) for a while or not (0) */

    litl_write_buffer_t **buffers; /**< An array of thread-specific buffers */
    litl_size_t nb_allocated_buffers; /**< A number of thread-specific buffers that are allocated */
} litl_trace_write_t;

/**
 * \ingroup litl_types_read
 * \brief A data structure for reading one event
 */
typedef struct {
    litl_tid_t tid; /**< A thread ID */
    litl_t *event; /**< A pointer to the read event */
} litl_read_t;

/**
 * \ingroup litl_types_read
 * \brief A data structure for reading thread-specific events
 */
typedef struct {
    litl_header_tids_t* tids; /**< An array of thread IDs*/

    litl_buffer_t buffer_ptr; /**< A pointer to the beginning of the buffer */
    litl_buffer_t buffer; /**< A pointer to the current position in the buffer */

    litl_offset_t offset; /**< An offset from the beginning of the buffer */
    litl_offset_t tracker; /**< An indicator of the end of the buffer, which equals to offset + buffer_size */

    litl_read_t cur_event; /**< The current event */
} litl_trace_read_thread_t;

/**
 * \ingroup litl_types_read
 * \brief A data structure for reading process-specific events
 */
typedef struct {
    litl_header_triples_t* triples; /**< An array of triples (fid, trace_size, offset)*/

    litl_header_t* header; /**< A pointer to the header*/
    litl_size_t header_size; /**< A header size */
    litl_buffer_t header_buffer_ptr; /**< A pointer to the beginning of the header buffer */
    litl_buffer_t header_buffer; /**< A pointer to the current position within the header buffer */

    litl_med_size_t nb_buffers; /**< A number of buffers */
    litl_trace_read_thread_t *buffers; /**< An array of buffers */

    int cur_index; /**< An index of the current thread */
    int initialized; /**< Indicates that the process was initialized */
} litl_trace_read_process_t;

/**
 * \ingroup litl_types_read
 * \brief A data structure for reading events from both regular trace files and
 *  archives of traces
 */
typedef struct {
    int f_handle; /**< A file handler */

    litl_header_t* header; /**< A pointer to the header */
    litl_size_t header_size; /**< A header size */
    litl_buffer_t header_buffer_ptr; /**< A pointer to the beginning of the header buffer */
    litl_buffer_t header_buffer; /**< A pointer to the current position in the header buffer */

    litl_med_size_t nb_traces; /**< A number of traces */
    litl_trace_read_process_t *traces; /**< An array of traces */
} litl_trace_read_t;

/**
 * \ingroup litl_types_merge
 * \brief A data structure for merging trace files into an archive of traces
 */
typedef struct {
    int f_handle; /**< A file handler */
    char* filename; /**< A file name */

    litl_med_size_t nb_traces; /**< A number of traces */
    char** traces_names; /**< An array of traces names */

    litl_buffer_t buffer_ptr; /**< A pointer to the beginning of the buffer */
    litl_buffer_t buffer; /**< A pointer to the current position in the buffer */
    litl_size_t buffer_size; /**< A buffer size */

    litl_offset_t general_offset; /**< An offset from the beginning of the trace file till the current position */
} litl_trace_merge_t;

/**
 * \ingroup litl_types_split
 * \brief A data structure for splitting an archive of traces
 */
typedef struct {
    int f_arch; /**< An archive handler */

    litl_size_t header_size; /**< A header size */
    litl_buffer_t header_buffer_ptr; /**< A pointer to the beginning of the header buffer */
    litl_buffer_t header_buffer; /**< A pointer to the current position within the header buffer */

    litl_data_t is_trace_archive; /**< Indicates whether a trace is a regular trace (=0) or an archive of traces (=1) */
    litl_med_size_t nb_traces; /**< A number of trace */
    litl_header_triples_t* triples; /**< An array of triples (fid, trace_size, offset) */

    litl_buffer_t buffer; /**< A pointer to the buffer */
    litl_size_t buffer_size; /**< A buffer size */
} litl_trace_split_t;

/*
 * Defining formats for printing data
 */
#ifdef __x86_64__
#define PRTIu32 "u"
#define PRTIu64 "lu"
#define PRTIx32 "x"
#define PRTIx64 "lx"
#elif defined __arm__
#define PRTIu32 "u"
#define PRTIu64 "u"
#define PRTIx32 "x"
#define PRTIx64 "x"
#endif

/*
 * For internal use only.
 * Computes the offset of MEMBER in structure TYPE
 */
#define __litl_offset_of(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

/*
 * For internal use only.
 * Computes the offset of the first parameter in an event
 */
#define LITL_BASE_SIZE __litl_offset_of(litl_t, parameters.regular.param)

#endif /* LITL_TYPES_H_ */
