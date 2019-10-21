/**
 * @file util.c
 * @brief General project-independent helper macros
 *
 */

#include <unistd.h>

#include "util.h"

bool file_exists(const char *filename) { return access(filename, F_OK) != -1; }

// Adapted from: https://stackoverflow.com/a/238609/6304441
int file_size(FILE *file) {
	// Seek to end of file
	fseek(file, 0, SEEK_END);
	// Get size
	int size = ftell(file);
	// Seek back to starting position
	fseek(file, 0, SEEK_SET);

  return size;
}
