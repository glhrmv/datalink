/**
 * @file util.h
 * @brief General project-independent helper macros
 *
 */

#pragma once

#include <stdio.h>
#include <stdbool.h>

#define DEFAULT_BAUDRATE B38400
#define DEFAULT_MSG_MAX_SIZE 512
#define DEFAULT_RETRIES 3
#define DEFAULT_TIMEOUT 3

#define MAX_SIZE 256

#define BIT(n) (0x01 << n)

/**
 * @brief Checks if file exists on system
 *
 * @param filename Path to file
 * @return bool True if file exists, false otherwise
 */
bool file_exists(const char *filename);

/**
 * @brief Gets the size of a file
 * 
 * @param file Desired file
 * @return int File size
 */
int file_size(FILE* file);
