/**
 * @file util.c
 * @brief General project-independent helper macros
 *
 */

#include <unistd.h>

#include "util.h"

bool file_exists(const char *file_name) { return access(file_name, F_OK) != -1; }

// Adapted from: https://stackoverflow.com/a/238609/6304441
int get_file_size(FILE *file) {
	// Seek to end of file
	fseek(file, 0, SEEK_END);
	// Get size
	int size = ftell(file);
	// Seek back to starting position
	fseek(file, 0, SEEK_SET);

  return size;
}

char *BCC2_error_generator(char *packet, int packet_size)
{
  char *copy = (char *)malloc(packet_size);
  memcpy(copy, packet, packet_size);
  int r = (rand() % 100) + 1;
  if (r <= BCC2ErrorPercentage)
  {
    int i = (rand() % (packet_size - 5)) + 4;
    unsigned char random_letter = (unsigned char)('A' + (rand() % 26));
    copy[i] = random_letter;
    printf("\n** BCC2 Changed **\n\n");
  }
  return copy;
}

char *BCC1_error_generator(char *packet, int packet_size)
{
  char *copy = (char *)malloc(packet_size);
  memcpy(copy, packet, packet_size);
  int r = (rand() % 100) + 1;
  if (r <= BCC1ErrorPercentage)
  {
    int i = (rand() % 3) + 1;
    unsigned char random_letter = (unsigned char)('A' + (rand() % 26));
    copy[i] = random_letter;
    printf("\n** BCC1 Changed **\n\n");
  }
  return copy;
}
