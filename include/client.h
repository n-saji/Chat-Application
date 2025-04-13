#ifndef CLIENT_H_
#define CLIENT_H_
#include "structures.h"

class client
{
public:
  client(char *port);
  network_info info_struct;
};

#endif