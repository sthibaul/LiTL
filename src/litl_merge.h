/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_MERGE_H_
#define LITL_MERGE_H_

#include "litl_types.h"

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
void litl_merge_file(const int file_id, const char * file_name_in);

#endif /* LITL_MERGE_H_ */
