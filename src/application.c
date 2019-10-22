/**
 * @file application.c
 * @brief The datalink program application layer source file
 *
 * This is the application layer of the project.
 *
 * It is also the starting point of the built program.
 *
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "application.h"

void set_config(config_t *config, const char **argv) {
  // Initialize pointers
  config->port = (char*) malloc(256);
  config->file_name = (char*) malloc(256);

  // Set the connection mode
  if (strcmp(argv[1], "send") == 0)
    config->ct = SEND;
  else if (strcmp(argv[1], "receive") == 0)
    config->ct = RECEIVE;
  else {
    printf("Invalid connection mode. Must be 'send' or 'receive'\n");
    exit(-1);
  }

  // Build the serial port file name from the port number given
  char str[64];
  sprintf(str, "/dev/%s", argv[2]);

  // Check if serial port exists
  if (!file_exists(str)) {
    printf("Invalid port number given. Serial port %s does not exist.\n", str);
    exit(-1);
  }

  // Set the serial port device file path
  strcpy(config->port, str);

  // If receiving, keep config file name as recognisable string
  strcpy(config->file_name, "null");

  // If receiving, we're done setting the config
  if (config->ct == RECEIVE)
    return;

  // Prompt user for name of file to be sent
  printf("Name of file to send ? ");
  memset(str, 0, sizeof(str));
  scanf("%[^\n]%*c", str);
    char *directory_file  = (char *) malloc(30);
    strcpy(directory_file, "files/");
    strcat(directory_file, str);

  // Check if file exists
  if (!file_exists(directory_file)) {
    printf("Invalid file name given. File %s does not exist.\n", str);
    exit(-1);
  }

  // Set the file name
  strcpy(config->file_name, str);
}

int run(const config_t *config) {
  // Set the data link layer struct
  link_layer_t *ll = (link_layer_t *)malloc(sizeof(link_layer_t));
  if (set_link_layer(ll, config->port, config->ct) < 0) {
    perror("set_link_layer");
    return -1;
  }

  // Establish connection
  if (llopen(ll) < 0)
    return -1;

  // Perform transfer
  if (config->ct == SEND) {
    if (send_file(ll, config->file_name) < 0)
      return -1;
  } else if (config->ct == RECEIVE) {
    if (receive_file(ll) < 0)
      return -1;
  }

  // Close connection
  return llclose(ll);
}

int send_file(link_layer_t *ll, const char *file_name) {
  // Open file
  char *directory_file = (char *) malloc(30);
  strcpy(directory_file, "files/");
  strcat(directory_file, file_name);
  FILE *file = fopen(directory_file, "rb");

  // TODO: get size of file to be sent,
  //       and allocate a character buffer with
  //       the same size
  int file_size = get_file_size(file);

  // TODO: send START control packet
  //       (with file size and name in value field)
  packet_t *packet =(packet_t *)malloc(sizeof(packet_t));
  packet->type = PACKET_TYPE_DATA;
  packet->file_name = file_name;
  packet->file_size = file_size;
  char *file_size_buf = (char *)malloc(sizeof(char));
  sprintf(file_size_buf, "%d", file_size);
  start_packet->file_size_buf = filesize_buf;
  send_control_packet(ll, start_packet);

  // TODO: send file chunks to llwrite
  char *file_chunk = malloc(MAX_SIZE);

  unsigned int read_bytes = 0; written_bytes = 0, i = 0;

  while((read_bytes = fread(file_chunk, sizeof(char), MAX_SIZE, file)) > 0) {
    if(!send_data_packet(ll, (i++) % 255, file_chunk, read_bytes)){
      free(file_chunk);
      return -1;
    }

    memset(file_chunk, 0, MAX_SIZE);
    written_bytes += read_bytes;
  }

  // TODO: free the character buffer

  // Close file
  fclose(file);

  // TODO: send END control packet
  packet->type = PACKET_TYPE_END;

  send_control_packet(ll, packet);

  return 0;
}

int receive_file(link_layer_t *ll) {
  // TODO: receive START control packet
  //       (with file size and name in value field)

  // TODO: create a file with the proper name

  // TODO: read from from llread
  //       (should read as many bytes as file size given)

  // TODO: create

  return 0;
}

int send_control_packet(link_layer_t *ll, const packet_t *packet) {
  // Calculate control packet size
  int packet_buf_size =
      5 + strlen(packet->file_size_buf) + strlen(packet->file_name);

  // Create the control packet buffer
  char packet_buf[packet_buf_size];
  unsigned int pos = 0;

  // Set packet type
  packet_buf[pos++] = packet->type;

  // Set file size field
  packet_buf[pos++] = FILE_SIZE_FIELD;
  packet_buf[pos++] = strlen(packet->file_size_buf);
  for (unsigned int i = 0; i < strlen(packet->file_size_buf); i++)
    packet_buf[pos++] = packet->file_size_buf[i];

  // Set file name field
  packet_buf[pos++] = FILE_NAME_FIELD;
  packet_buf[pos++] = strlen(packet->file_name);
  for (unsigned int i = 0; i < strlen(packet->file_name); i++)
    packet_buf[pos++] = packet->file_name[i];

  // Write it to the serial port
  if (llwrite(ll, packet_buf, packet_buf_size) < 0)
    return -1;

  free(packet_buf);

  return 0;
}

int receive_control_packet(link_layer_t *ll, packet_t *packet) {
  // Receive packet
  char *packet_buf = NULL;
  int packet_buf_size = llread(ll, packet_buf);
  if (packet_buf_size < 0) {
    printf("Could not read from link layer.\n");
    return -1;
  }

  // Get packet type
  unsigned int pos = 0;
  packet->type = packet_buf[pos++];

  // Go through packet fields
  while (true) {
    switch (packet_buf[pos++]) {
    case FILE_SIZE_FIELD: {
      unsigned int octets = (unsigned int)packet_buf[pos++];

      char *length = malloc(octets);
      memcpy(length, &packet_buf[pos], octets);

      packet->file_size = atoi(length);
      free(length);

      break;
    }
    case FILE_NAME_FIELD: {
      unsigned char octets = (unsigned char)packet_buf[pos++];

      memcpy(packet->file_name, &packet_buf[pos], octets);

      break;
    }
    default:
      return -1;
    }
  }

  return 0;
}

int sendDataPackage(link_layer_t *ll, int N, const char* buffer, int length) {
	unsigned char C = PACKET_TYPE_DATA;
	unsigned char L2 = length / 256;
	unsigned char L1 = length % 256;

	// calculate package size
	unsigned int packageSize = 4 + length;

	// allocate space for package header and file chunk
	unsigned char* package = (unsigned char*) malloc(packageSize);

	// build package header
	package[0] = C;
	package[1] = N;
	package[2] = L2;
	package[3] = L1;

	// copy file chunk to package
	memcpy(&package[4], buffer, length);

	// write package
	if (!llwrite(ll, package, packageSize)) {
		printf(
				"ERROR: Could not write to link layer while sending data package.\n");
		free(package);

		return -1;
	}

	free(package);

	return 0;
}

int receiveDataPackage(link_layer_t *ll, int* N, char** buf, int* length) {
	unsigned char* package;

	// read package from link layer
	ui size = llread(fd, &package);
	if (size < 0) {
		printf(
				"ERROR: Could not read from link layer while receiving data package.\n");
		return 0;
	}

	int C = package[0];
	*N = (unsigned char) package[1];
	int L2 = package[2];
	int L1 = package[3];

	// assert package is a data package
	if (C != CTRL_PKG_DATA) {
		printf("ERROR: Received package is not a data package (C = %d).\n", C);
		return 0;
	}

	// calculate size of the file chunk contained in the read package
	*length = 256 * L2 + L1;

	// allocate space for that file chunk
	*buf = malloc(*length);

	// copy file chunk to the buffer
	memcpy(*buf, &package[4], *length);

	// destroy the received package
	free(package);

	return 1;
}

int main(int argc, const char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port device name>\n", argv[0]);
    return 1;
  }

  // Set the program config struct
  config_t *config = (config_t *)malloc(sizeof(config_t));
  set_config(config, argv);

  // We're good to go
  return run(config);
}
