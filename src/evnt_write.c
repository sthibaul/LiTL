/*
 * evnt_write.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "evnt_write.h"

static uint64_t* buffer_ptr;
uint64_t* buffer_cur;

static uint32_t buffer_size = 32 * 1024; // 32 KB
static buffer_flags buffer_flush_flag = EVNT_BUFFER_FLUSH;
static thread_flags thread_safe_flag = EVNT_NOTHREAD_SAFE;
static FILE* ftrace;

/*
 * This function returns the size of event depending on the number of arguments

 static uint64_t _get_event_size(uint64_t nb_args) {
 // 4 is the size of tid, time, code, and nb_args
 return nb_args + 4;
 }*/

/*
 * This function computes the size of data in buffer
 */
static uint32_t _get_buffer_size(uint64_t* buffer_ptr, uint64_t* buffer_cur) {
    return sizeof(uint64_t) * ((uint64_t *) buffer_cur - (uint64_t *) buffer_ptr);
}

/*
 * This function returns the current time in ns
 */
static uint64_t _get_time() {
    // TODO: implement such function using rdtscll, which will return time in clock cycles
    struct timespec tp;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    return ((uint64_t) tp.tv_nsec + (uint64_t) (1000000000 * (tp.tv_sec)));
}

/*
 * This function initializes the trace
 */
void trace_init(char* file_path, buffer_flags flush_flag, thread_flags thread_flag, uint32_t buf_size) {
    int ok;
    void *vp;

    // TODO: add a checker, so if any of the arguments is not given, then the default value will be taken

    // allocate memory to be sure that it is aligned
    // the size of buffer is slightly bigger than it was required, because two or three additional events are added
    //      before and after tracing, and also after the buffer was flushed
    ok = posix_memalign(&vp, sizeof(uint64_t), buf_size + 3 * sizeof(evnt));
    if (ok != 0)
        perror("Could not allocate memory for the buffer!");

    // set variables
    buffer_ptr = vp;
    buffer_cur = buffer_ptr;
    buffer_size = buf_size;
    buffer_flush_flag = flush_flag;
    thread_safe_flag = thread_flag;

    // TODO: perhaps, it is better to touch each block in buffer_ptr in order to load it

    // check whether the trace file can be opened
    if (!(ftrace = fopen(file_path, "w+")))
        perror("Could not open the trace file!");

    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 4;

    // write time and the EVNT_TRACE_START (= 0) code to indicate the start of tracing
    // TODO: how to set the thread ID?
    ((evnt *) buffer_cur)->tid = 0;
    ((evnt *) buffer_cur)->time = _get_time();
    ((evnt *) buffer_cur)->code = EVNT_TRACE_START;
    ((evnt *) buffer_cur)->nb_args = 0;
    buffer_cur += evnt_size;
}

/*
 * This function finalizes the trace
 */
void trace_fin() {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 4;

    // write time and the EVNT_TRACE_END (= 2) code to indicate the end of tracing
    // TODO: how to set the thread ID?
    ((evnt *) buffer_cur)->tid = 0;
    ((evnt *) buffer_cur)->time = _get_time();
    ((evnt *) buffer_cur)->code = EVNT_TRACE_END;
    ((evnt *) buffer_cur)->nb_args = 0;
    buffer_cur += evnt_size;

    if (fwrite(buffer_ptr, _get_buffer_size(buffer_ptr, buffer_cur), 1, ftrace) != 1)
        perror("Could not write measured data to the trace file!");

    fclose(ftrace);
    free(buffer_ptr);
}

/*
 * This function writes the recorded events from the buffer to the file
 */
void buffer_flush() {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 4;

    // write time and the EVNT_BUFFER_FLUSHED (= 1) code to indicate the buffer flush start
    // TODO: how to set the thread ID?
    ((evnt *) buffer_cur)->tid = 0;
    ((evnt *) buffer_cur)->time = _get_time();
    ((evnt *) buffer_cur)->code = EVNT_BUFFER_FLUSHED;
    ((evnt *) buffer_cur)->nb_args = 0;
    buffer_cur += evnt_size;

    if (fwrite(buffer_ptr, _get_buffer_size(buffer_ptr, buffer_cur), 1, ftrace) != 1)
        perror("Flushing the buffer. Could not write measured data to the trace file!");

    buffer_cur = buffer_ptr;
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(uint64_t code) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 4;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 0;

        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe0(code);
    }
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(uint64_t code, uint64_t param1) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 5;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 1;
        ((evnt *) buffer_cur)->args[1] = param1;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe1(code, param1);
    }
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(uint64_t code, uint64_t param1, uint64_t param2) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 6;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 2;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe2(code, param1, param2);
    }
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 7;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 3;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe3(code, param1, param2, param3);
    }
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 8;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 4;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe4(code, param1, param2, param3, param4);
    }
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 9;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 5;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        ((evnt *) buffer_cur)->args[5] = param5;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe5(code, param1, param2, param3, param4, param5);
    }
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 10;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 6;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        ((evnt *) buffer_cur)->args[5] = param5;
        ((evnt *) buffer_cur)->args[6] = param6;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe6(code, param1, param2, param3, param4, param5, param6);
    }
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 11;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 7;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        ((evnt *) buffer_cur)->args[5] = param5;
        ((evnt *) buffer_cur)->args[6] = param6;
        ((evnt *) buffer_cur)->args[7] = param7;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe7(code, param1, param2, param3, param4, param5, param6, param7);
    }
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 12;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 8;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        ((evnt *) buffer_cur)->args[5] = param5;
        ((evnt *) buffer_cur)->args[6] = param6;
        ((evnt *) buffer_cur)->args[7] = param7;
        ((evnt *) buffer_cur)->args[8] = param8;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe8(code, param1, param2, param3, param4, param5, param6, param7, param8);
    }
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8, uint64_t param9) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = 13;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt *) buffer_cur)->tid = 0;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 9;
        ((evnt *) buffer_cur)->args[1] = param1;
        ((evnt *) buffer_cur)->args[2] = param2;
        ((evnt *) buffer_cur)->args[3] = param3;
        ((evnt *) buffer_cur)->args[4] = param4;
        ((evnt *) buffer_cur)->args[5] = param5;
        ((evnt *) buffer_cur)->args[6] = param6;
        ((evnt *) buffer_cur)->args[7] = param7;
        ((evnt *) buffer_cur)->args[8] = param8;
        ((evnt *) buffer_cur)->args[9] = param9;
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_probe9(code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }
}

/*
 * This function records an event in a raw state.
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t code, uint64_t size, void* data) {
    // evnt_size represents the number of events' properties such as tid, time, code, nb_args, and args
    int evnt_size = size / sizeof(uint64_t) + 4;

    if (_get_buffer_size(buffer_ptr, buffer_cur) < buffer_size) {
        // TODO: how to set the thread ID?
        ((evnt_raw *) buffer_cur)->tid = 0;
        ((evnt_raw *) buffer_cur)->time = _get_time();
        ((evnt_raw *) buffer_cur)->code = code;
        ((evnt_raw *) buffer_cur)->size = size;
        ((evnt_raw *) buffer_cur)->data = data;
        // sizeof(uint64_t) * 4 is the size of tid, time, code, and size, where size presents the number of arguments
        //   of type uint64_t
        buffer_cur += evnt_size;
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        buffer_flush();
        evnt_raw_probe(code, size, data);
    }
}
