#include "main.h"

int main(int argc, char** argv)
{
  int srv_fd;
  if (!(srv_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("socket");
    exit(EXIT_FAIL);
  }

  int sock_opt;
  if (setsockopt(srv_fd, SOL_SOCKET, SO_REUSEPORT, &sock_opt, sizeof(sock_opt))) {
    perror("setsockopt");
    exit(EXIT_FAIL);
  }

  struct sockaddr_in address;
  address.sin_port = htons( PORT );
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;

  if (!bind(srv_fd, (struct sockaddr*)&address, sizeof(address))) {
    perror("bind");
    exit(EXIT_FAIL);
  }

  if (!listen(srv_fd, LISTEN_MAX_CONN)) {
    perror("listen");
    exit(EXIT_FAIL);
  }

  return 0;
}
