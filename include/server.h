#ifndef SERVER_H_
#define SERVER_H_
#include "structures.h"

class server
{
public:
  network_info info_struct;
  server(char *port);
};

#endif
