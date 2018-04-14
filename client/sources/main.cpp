#include "main.h"

int main(int argc, char** argv)
{
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

  while(1) {
    // something should be here.. ;)
  }

	return 0;
}
