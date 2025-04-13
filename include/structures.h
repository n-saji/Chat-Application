#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <string.h>
#include <list>
#include <queue>

struct blocked_clients
{
  blocked_clients() : listen_port_num(-1)
  {
    memset(&host_name, 0, sizeof(host_name));
    memset(&ip_address, 0, sizeof(ip_address));
  }
  char host_name[1024];
  char ip_address[50];
  int listen_port_num;
};

struct buffer_info
{
  buffer_info()
  {
    memset(&fr, 0, sizeof(fr));
    memset(&message, 0, sizeof(message));
    memset(&destination_ip_address, 0, sizeof(destination_ip_address));
  }
  char destination_ip_address[32];
  char message[1024];
  char fr[32];
};

struct socket_info
{
  socket_info() : message_count_sent(0), received_message_count(0), list_id(-1), fd(-1), port_number(-1)
  {
    memset(&host_name, 0, sizeof(host_name));
    memset(&ip_address, 0, sizeof(ip_address));
    memset(&status, 0, sizeof(status));
  }
  int fd;
  int list_id;
  int port_number;

  // for messages statistics
  int message_count_sent;
  int received_message_count;

  char host_name[40];
  char ip_address[20];
  char status[16];

  std::list<blocked_clients> blocked_clients_list;
  std::queue<buffer_info> buffer;
};

struct network_info
{
  network_info() : num_clients(0)
  {
    memset(&ip_address, 0, sizeof(ip_address));
    memset(&port_number, 0, sizeof(port_number));
  }
  std::list<socket_info> clients;
  std::list<blocked_clients> block_list;

  char ip_address[50];
  char port_number[1024];

  int listener_number;
  int num_clients;
};

#endif