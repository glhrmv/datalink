/**
 * @file util.h
 * @brief General project-independent helper macros
 *
 */

#pragma once

#include <stdio.h>
#include <stdbool.h>

#define BIT(n) (0x01 << n)
#define TRUE 1
#define FALSE 0
#define BCC1ErrorPercentage 2
#define BCC2ErrorPercentage 2

/**
 * @brief Checks if file exists on system
 *
 * @param file_name Path to file
 * @return bool True if file exists, false otherwise
 */
bool file_exists(const char *file_name);

/**
 * @brief Gets the size of a file
 * 
 * @param file Desired file pointer
 * @return int File size
 */
int get_file_size(FILE* file);

char *BCC2_error_generator( char *packet, int sizePacket);

char *BCC1_error_generator( char *packet, int sizePacket);
