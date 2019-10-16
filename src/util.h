/**
 * @file util.h
 * @brief General project-independent helper macros
 *
 */

#pragma once

#include <stdbool.h>

#define TRUE 1
#define FALSE 0

#define MAX_SIZE 256

#define BIT(n) (0x01 << n)

/**
 * @brief Checks if file exists on system
 * 
 * @param filename Path to file
 * @return bool True if file exists, false otherwise
 */
bool file_exists(const char *filename);
