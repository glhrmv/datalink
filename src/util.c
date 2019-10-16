/**
 * @file util.c
 * @brief General project-independent helper macros
 *
 */

#include <unistd.h>

#include "util.h"

bool file_exists(const char *filename) { return access(filename, F_OK) != -1; }
