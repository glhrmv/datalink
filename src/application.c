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
  config->port = (char *)malloc(256);
  config->file_name = (char *)malloc(256);

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
  char *directory_file = (char *)malloc(30);
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
    if (send_file(ll, config->file_name) != 0)
      return -1;
  } else if (config->ct == RECEIVE) {
    if (receive_file(ll) != 0)
      return -1;
  }

  // Close connection
  return llclose(ll);
}

int send_file(link_layer_t *ll, char *file_name) {
  // Open file
  char *directory_file = (char *)malloc(30);
  strcpy(directory_file, "files/");
  strcat(directory_file, file_name);
  FILE *file = fopen(directory_file, "rb");

  // Get size of file to be sent,
  // and allocate a character buffer with
  // the same size
  int file_size = get_file_size(file);

  // Send START control packet
  // (with file size and name in value field)
  packet_t *packet = (packet_t *)malloc(sizeof(packet_t));
  packet->type = PACKET_TYPE_DATA;
  packet->file_name = file_name;
  packet->file_size = file_size;
  char file_size_buf[sizeof(int) * 3 + 2];
  snprintf(file_size_buf, sizeof file_size_buf, "%d", file_size);
  packet->file_size_buf = file_size_buf;

  if(send_control_packet(ll, packet) != 0){
    printf("Error: Start control packet not sent.\n");
    return -1;
  }

  // Send file chunks to llwrite
  char *file_chunk = (char *)malloc(MAX_SIZE);

  unsigned int read_bytes = 0, i = 0;

  while ((read_bytes = fread(file_chunk, sizeof(char), MAX_SIZE, file)) > 0) {
    printf("Sending file chunk: 0x%08x\n", *file_chunk);
    if (send_data_packet(ll, (i++) % 255, file_chunk, read_bytes) != 0) {
      free(file_chunk);
      return -1;
    }

    memset(file_chunk, 0, MAX_SIZE);
  }

  free(file_chunk);

  // Close file
  fclose(file);

  packet->type = PACKET_TYPE_END;

  if(send_control_packet(ll, packet) != 0){
    printf("Error: End control packet no send.\n");
    return -1;
  }

  return 0;
}

int receive_file(link_layer_t *ll) {
  // Receive START control packet
  // (with file size and name in value field)
  packet_t *packet = (packet_t *)malloc(sizeof(packet_t));

  if (receive_control_packet(ll, packet) != 0) {
    printf("Error: Control packet not received\n");
    return -1;
  }

  printf("Receiving file: %s ", packet->file_name);
  printf("(size: %d)\n", packet->file_size);

  // Create a file with the proper name
  FILE *file_created = fopen(packet->file_name, "wb");
  if (file_created == NULL) {
    printf("Error: Could not create file.\n");
    return -1;
  }

  // Read from from llread
  // (should read as many bytes as file size given)
  int file_read_so_far = 0, n = -1;
  while (file_read_so_far != packet->file_size) {
    int last_n = n;
    char *file_buf;
    int length = 0;

    if (receive_data_packet(ll, &n, &file_buf, &length) != 0) {
      printf("Error: Data packet not received.\n");
      free(file_buf);
      return -1;
    }
    if (n != 0 && last_n + 1 != n) {
      printf("Received wrong sequence number.\n");
      free(file_buf);
    }
    else{
    printf("Rceived data packet.\n");
    fwrite(file_buf, sizeof(char), length, file_created);
    free(file_buf);

    file_read_so_far += length;
  }
}

  printf("All file chunks received!\n");

  packet_t *end_packet = (packet_t *)malloc(sizeof(packet_t));
  if (receive_control_packet(ll, end_packet) != 0) {
    printf("Error: End control packet not received\n");
    return -1;
  }

  if (end_packet->type != PACKET_TYPE_END) {
    printf("Error: END control packet not received.\n");
    return -1;
  }

  if (packet->file_size != get_file_size(file_created)) {
    printf("Got WRONG amount of bytes :(\n");
    return -1;
  }

  if(fclose(file_created) != 0){
    printf("Error: Closing file created.\n");
    return -1;
  }

  printf("File received! :)\n");
  return 0;
}

int send_control_packet(link_layer_t *ll, const packet_t *packet) {
  printf("Creating control packet...\n");
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


  printf("Sent control packet.\n");

  return 0;
}

int receive_control_packet(link_layer_t *ll, packet_t *packet) {
  // Receive packet
  char *packet_buf;
  int packet_buf_size = llread(ll, &packet_buf);
  if (packet_buf_size < 0) {
    printf("Could not read from link layer.\n");
    return -1;
  }

  // Get packet type
  unsigned int pos = 0;
  packet->type = packet_buf[pos++];

  // Go through packet fields
  for(int i = 0; i < 2;i++){
    switch (packet_buf[pos++]) {
      case FILE_SIZE_FIELD: {
        unsigned int octets = (unsigned int)packet_buf[pos++];

        char *length = (char *)malloc(octets);
        memcpy(length, &packet_buf[pos], octets);

        packet->file_size = atoi(length);
        free(length);
        pos += octets;
        break;
      }
      case FILE_NAME_FIELD: {
        packet->file_name = &packet_buf[++pos];

        //memcpy(&packet->file_name, &packet_buf[pos], octets);

        break;
      }
    }
  }

  return 0;
}

int send_data_packet(link_layer_t *ll, int N, const char *buffer, int length) {
  printf("Creating a data packet to send...\n");

  // calculate package size
  unsigned int packet_size = 4 + length;

  // allocate space for package header and file chunk
  unsigned char *packet = (unsigned char *)malloc(packet_size);

  // build package header
  packet[0] = PACKET_TYPE_DATA;
  packet[1] = N;
  packet[2] = length/256;
  packet[3] = length%256;
  printf("Sending packet: %d \n",packet[1]);

  // copy file chunk to package
  memcpy(&packet[4], buffer, length);

  // write package
  if (llwrite(ll, packet, packet_size) != 0) {
    printf("ERROR: Could not write to link layer while sending data package.\n");
    free(packet);

    return -1;
  }

  free(packet);

  return 0;
}

int receive_data_packet(link_layer_t *ll, int *n, char **buf, int *length) {
  printf("Receiving data packet...\n");
  unsigned char *packet;

  // read packet from link layer
  if (llread(ll, &packet) != 0) {
    printf("Error reading packet.\n");
    return -1;
  }

  printf("Received packet: %d\n", (int) packet[1]);

  int C = packet[0];
  *n = (unsigned char)packet[1];
  int L2 = packet[2];
  int L1 = packet[3];

  // assert packet is a data packet
  if (C != PACKET_TYPE_DATA) {
    printf("ERROR: Packet is not a data packet (C = %d).\n", C);
    return -1;
  }

  // File size of chunk in packet
  *length = 256 * L2 + L1;

  // Allocate space for that file chunk
  *buf = (char *)malloc(*length);

  // copy file chunk to the buffer
  memcpy(*buf, &packet[4], *length);

  // destroy the received packet
  free(packet);

  printf("Data packet received.\n");

  return 0;
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
