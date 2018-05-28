#include <stdio.h>
#include "client.h"

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  Client client;

  // Initialize everything in the client class
  client.init();

  // Wait until the client is working
  while(client.run());

  return 0;
}
