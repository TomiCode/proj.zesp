#include "main.h"
#include "client.h"

void receive_thread(int client_socket)
{
  uint8_t buffer[1024] = {0};
  size_t recv_size = 0;

  while(1) {
    recv_size = recv(client_socket, buffer, 512, 0);
    printf("Recv content from server size: %zu.\n", recv_size);
  }
}

void print_hex(char * ptr, size_t len)
{
  for(size_t i = 0; i < len; i++)
    printf("%x ", *(ptr++));
  printf("\n");
}

int main(int argc, char** argv)
{
  Client client;

  client.init();
  while(client.run());
  
  return 0; // Skip all other things for now.

  int client_socket = 0;
  if (!(client_socket = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("client socket");
    return -1;
  }

  struct sockaddr_in address = {0};
  address.sin_family = AF_INET;
  address.sin_port = htons(CONNECT_PORT);
  if (!inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)) {
    printf("Address is invalid or not supported.\n");
    return -1;
  }

  if (connect(client_socket, (struct sockaddr*)&address, sizeof(address))) {
    printf("Cannot connect.\n");
    return -1;
  }

  std::thread recv_thread(receive_thread, client_socket);
  char *stdin_line = NULL;
  size_t line_len = 0;
  ssize_t read_count = 0;

  while(1) {
    read_count = getline(&stdin_line, &line_len, stdin);
    if (read_count == -1) break;

    printf("Read -> %s.", stdin_line);
  }
  free(stdin_line);

  return 0;
}
