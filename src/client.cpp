#include "../include/client.h"
#include "../include/logger.h"

#include <arpa/inet.h>

#include <iostream>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include <string>

#include <unistd.h>

#define STD_IN 0
using namespace std;

void print_error_log(const char *error_message)
{
  cse4589_print_and_log("[%s:ERROR]\n", error_message);
  cse4589_print_and_log("[%s:END]\n", error_message);
}

bool validate_server_address(char *server_ip_address, int p)
{
  struct sockaddr_in ip_address;

  ip_address.sin_family = AF_INET;
  ip_address.sin_port = htons(p);

  if (server_ip_address == NULL)
  {
    return false;
  }

  if (inet_pton(AF_INET, server_ip_address, &ip_address.sin_addr) != 1)
  {
    return false;
  }

  return true;
}

client::client(char *port)
{
  struct hostent *host_entity;
  char host_name[1024];

  strcpy(info_struct.port_number, port);

  if (gethostname(host_name, 1024) < 0)
  {
    cerr << "failed to fetch host name\n";
    exit(1);
  }

  if ((host_entity = gethostbyname(host_name)) == NULL)
  {
    cerr << "host name is empty\n";
    exit(1);
  }

  struct in_addr **host_address_list = (struct in_addr **)host_entity->h_addr_list;

  for (int i = 0;
       host_address_list[i] != NULL;
       ++i)
  {
    strcpy(info_struct.ip_address, inet_ntoa(*host_address_list[i]));
  }

  if ((info_struct.listener_number = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    cerr << "error creating socket\n";
    exit(1);
  }

  struct sockaddr_in client_socket_address;

  memset(&client_socket_address, 0, sizeof(client_socket_address));

  client_socket_address.sin_family = AF_INET;

  client_socket_address.sin_port = htons(atoi(port));

  client_socket_address.sin_addr = *((struct in_addr *)host_entity->h_addr);

  if (::bind(info_struct.listener_number,
             (struct sockaddr *)&client_socket_address,
             sizeof(client_socket_address)) < 0)
  {
    cerr << "error binding to socket\n";
    exit(1);
  }

  char user_input[1024];

  while (true)
  {
    memset(&user_input, 0, sizeof(user_input));

    read(STD_IN, user_input, 1024);

    user_input[strlen(user_input) - 1] = '\0';

    if (strcmp(user_input, "AUTHOR") == 0)
    {
      cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
      cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "pa1-nsaji-atembulk");
      cse4589_print_and_log("[AUTHOR:END]\n");
    }

    else if (strcmp(user_input, "IP") == 0)
    {
      cse4589_print_and_log("[IP:SUCCESS]\n");
      cse4589_print_and_log("IP:%s\n", info_struct.ip_address);
      cse4589_print_and_log("[IP:END]\n");
    }

    else if (strcmp(user_input, "PORT") == 0)
    {
      cse4589_print_and_log("[PORT:SUCCESS]\n");
      cse4589_print_and_log("PORT:%s\n", info_struct.port_number);
      cse4589_print_and_log("[PORT:END]\n");
    }

    else if (strcmp(user_input, "LIST") == 0)
    {
      int i = 1;
      cse4589_print_and_log("[LIST:SUCCESS]\n");

      info_struct.clients.sort([](const socket_info &si1, const socket_info &si2)
                               { return si1.port_number < si2.port_number; });
      for (const auto &socket : info_struct.clients)
      {
        if (strcmp(socket.status, "logged-in") == 0)
          cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i++, socket.host_name,
                                socket.ip_address, socket.port_number);
      }

      cse4589_print_and_log("[LIST:END]\n");
    }

    else if (strncmp(user_input, "LOGIN", 5) == 0)
    {
      char *server_ip;
      char *server_port;
      strtok(user_input, " ");
      server_ip = strtok(NULL, " ");
      server_port = strtok(NULL, " ");

      bool valid_port = true;

      if (server_port == NULL)
      {
        print_error_log("LOGIN");
        continue;
      }
      
      for (int i = 0; i != strlen(server_port); ++i)
      {
        if (server_port[i] >= '0' && server_port[i] <= '9')
        {
          continue;
        }
        else
        {
          print_error_log("LOGIN");
          valid_port = false;
          break;
        }
      }
      if (!valid_port)
        continue;

      int port = atoi(server_port);
      if (port < 0 || port > 65535)
      {
        print_error_log("LOGIN");
        continue;
      }

      if (!validate_server_address(server_ip, port))
      {
        print_error_log("LOGIN");
        continue;
      }
      else
      {
        struct addrinfo temp;
        struct addrinfo *result;

        memset(&temp, 0, sizeof(temp));

        temp.ai_family = AF_INET;
        temp.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(server_ip, server_port, &temp, &result) != 0)
        {
          print_error_log("LOGIN");
          continue;
        }
        else
        {
          if ((info_struct.listener_number = socket(AF_INET, SOCK_STREAM, 0)) < 0)
          {
            print_error_log("LOGIN");
            continue;
          }

          struct sockaddr_in destination_address;
          memset(&destination_address, 0, sizeof(destination_address));

          destination_address.sin_family = AF_INET;
          destination_address.sin_port = htons(port);
          destination_address.sin_addr.s_addr = inet_addr(server_ip);

          if ((connect(info_struct.listener_number, (struct sockaddr *)&destination_address, sizeof(struct sockaddr))) < 0)
          {
            print_error_log("LOGIN");
            continue;
          }

          char client_port[8];
          memset(client_port, 0, sizeof(client_port));
          strcat(client_port, info_struct.port_number);
          if (send(info_struct.listener_number, client_port, strlen(client_port), 0) < 0)
          {
            cerr << "error connecting to server with port: " << client_port << endl;
          }

          char buf[1024];
          for (;;)
          {

            memset(&buf, 0, sizeof(buf));
            fd_set read_fds;

            FD_ZERO(&read_fds);
            FD_SET(STD_IN, &read_fds);
            FD_SET(info_struct.listener_number, &read_fds);

            int fd_max = info_struct.listener_number;
            select(fd_max + 1, &read_fds, NULL, NULL, NULL);
            if (FD_ISSET(STD_IN, &read_fds))
            {
              read(STD_IN, buf, 1024);
              buf[strlen(buf) - 1] = '\0';

              if (strcmp(buf, "AUTHOR") == 0)
              {
                cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
                cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "pa1-nsaji-atembulk");
                cse4589_print_and_log("[AUTHOR:END]\n");
              }

              else if (strcmp(buf, "IP") == 0)
              {
                cse4589_print_and_log("[IP:SUCCESS]\n");
                cse4589_print_and_log("IP:%s\n", info_struct.ip_address);
                cse4589_print_and_log("[IP:END]\n");
              }

              else if (strcmp(buf, "PORT") == 0)
              {
                cse4589_print_and_log("[PORT:SUCCESS]\n");
                cse4589_print_and_log("PORT:%s\n", info_struct.port_number);
                cse4589_print_and_log("[PORT:END]\n");
              }

              else if (strcmp(buf, "LIST") == 0)
              {
                cse4589_print_and_log("[LIST:SUCCESS]\n");
                int i = 0;
                info_struct.clients.sort([](const socket_info &si1, const socket_info &si2)
                                         { return si1.port_number < si2.port_number; });
                for (const auto &socket : info_struct.clients)
                {
                  if (strcmp(socket.status, "logged-in") == 0)
                    cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", ++i, socket.host_name,
                                          socket.ip_address, socket.port_number);
                }
                cse4589_print_and_log("[LIST:END]\n");
              }

              else if (strcmp(buf, "REFRESH") == 0)
              {
                strcat(buf, " ");
                strcat(buf, info_struct.ip_address);
                if (send(info_struct.listener_number, buf, strlen(buf), 0) < 0)
                {
                  print_error_log("REFRESH");
                }
                cse4589_print_and_log("[REFRESH:SUCCESS]\n");
                cse4589_print_and_log("[REFRESH:END]\n");
              }

              else if (strncmp(buf, "SEND", 4) == 0)
              {
                char send_message[1024];
                memset(&send_message, 0, sizeof(send_message));
                strcpy(send_message, buf);
                char *arg[3];
                arg[0] = strtok(buf, " ");
                for (int i = 1; i != 3; ++i)
                {
                  arg[i] = strtok(NULL, " ");
                }

                bool isval = false;
                for (const auto &socket : info_struct.clients)
                {
                  if (strcmp(socket.ip_address, arg[1]) == 0)
                    isval = true;
                }

                if (!isval || send(info_struct.listener_number, send_message, strlen(send_message), 0) < 0)
                {
                  print_error_log("SEND");
                  continue;
                }
                cse4589_print_and_log("[SEND:SUCCESS]\n");
                cse4589_print_and_log("[SEND:END]\n");
              }
              else if (strncmp(buf, "BROADCAST", 9) == 0)
              {
                if (send(info_struct.listener_number, buf, strlen(buf), 0) < 0)
                {
                  print_error_log("BROADCAST");
                }
                cse4589_print_and_log("[BROADCAST:SUCCESS]\n");
                cse4589_print_and_log("[BROADCAST:END]\n");
              }
              else if (strncmp(buf, "BLOCK", 5) == 0)
              {
                char temp_buf[1024];
                memset(&temp_buf, 0, sizeof(temp_buf));
                strcpy(temp_buf, buf);
                strtok(temp_buf, " ");
                char *block_ip = strtok(NULL, " ");

                if (block_ip == NULL)
                {
                  print_error_log("BLOCK");
                  continue;
                }

                bool isval = false;
                bool isblocked = false;
                blocked_clients b;
                for (const auto &socket : info_struct.clients)
                {
                  if (strcmp(socket.ip_address, block_ip) == 0)
                  {
                    isval = true;
                    b.listen_port_num = socket.port_number;
                    strcpy(b.host_name, socket.host_name);
                    strcpy(b.ip_address, socket.ip_address);
                    break;
                  }
                }
                for (const auto &blocked_client : info_struct.block_list)
                {
                  if (strcmp(block_ip, blocked_client.ip_address) == 0)
                  {
                    isblocked = true;
                    break;
                  }
                }

                if (!isval || isblocked)
                {
                  print_error_log("BLOCK");
                  continue;
                }
                if (send(info_struct.listener_number, buf, strlen(buf), 0) < 0)
                {
                  print_error_log("BLOCK");
                  continue;
                }
                else
                {
                  info_struct.block_list.push_back(b);
                }
                cse4589_print_and_log("[BLOCK:SUCCESS]\n");
                cse4589_print_and_log("[BLOCK:END]\n");
              }
              else if (strncmp(buf, "UNBLOCK", 7) == 0)
              {
                char *arg[2];
                char temp_buf[1024];
                memset(&temp_buf, 0, sizeof(temp_buf));
                strcpy(temp_buf, buf);
                arg[0] = strtok(temp_buf, " ");
                arg[1] = strtok(NULL, " ");

                bool valid = false;
                for (auto block_it = info_struct.block_list.begin(); block_it != info_struct.block_list.end();)
                {
                  if (strcmp(block_it->ip_address, arg[1]) == 0)
                  {
                    block_it = info_struct.block_list.erase(block_it);
                    valid = true;
                    break;
                  }
                  else
                  {
                    ++block_it;
                  }
                }

                if (!valid || send(info_struct.listener_number, buf, strlen(buf), 0) < 0)
                {
                  print_error_log("UNBLOCK");
                  continue;
                }
                cse4589_print_and_log("[UNBLOCK:SUCCESS]\n");
                cse4589_print_and_log("[UNBLOCK:END]\n");
              }
              else if (strcmp(buf, "LOGOUT") == 0)
              {
                cse4589_print_and_log("[LOGOUT:SUCCESS]\n");
                close(info_struct.listener_number);
                cse4589_print_and_log("[LOGOUT:END]\n");
                break;
              }
              else if (strcmp(buf, "EXIT") == 0)
              {
                close(info_struct.listener_number);
                cse4589_print_and_log("[EXIT:SUCCESS]\n");
                cse4589_print_and_log("[EXIT:END]\n");
                exit(0);
              }
            }
            else
            {
              char msg[1024];
              memset(&msg, 0, sizeof(msg));
              int recvbytes;
              if ((recvbytes = recv(info_struct.listener_number, msg, sizeof(msg), 0)) <= 0)
              {
                cout << "error receiving from server\n";
              }

              char *arg_zero = strtok(msg, " ");

              if (FD_ISSET(info_struct.listener_number, &read_fds))
              {

                if (strcmp(arg_zero, "SEND") == 0)
                {
                  cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                  char *arg[4];
                  arg[1] = strtok(NULL, " ");
                  arg[2] = strtok(NULL, " ");
                  arg[3] = strtok(NULL, "");
                  cse4589_print_and_log("msg from:%s\n[msg]:%s\n", arg[1], arg[3]);
                  cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                }
                else if (strcmp(arg_zero, "BROADCAST") == 0)
                {
                  cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                  char *arg[3];
                  arg[1] = strtok(NULL, " ");
                  arg[2] = strtok(NULL, "");
                  cse4589_print_and_log("msg from:%s\n[msg]:%s\n", arg[1], arg[2]);
                  cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                }
                else if (strcmp(arg_zero, "LOGIN") == 0)
                {
                  info_struct.clients.clear();
                  while (true)
                  {
                    char *list_msg[3];

                    list_msg[0] = strtok(NULL, " ");
                    char mesg[512];
                    char messag[4096];
                    memset(&mesg, 0, sizeof(mesg));
                    while (list_msg[0] != NULL && strcmp(list_msg[0], "BUFFER") == 0)
                    {
                      char original_messag[4096];
                      memset(&original_messag, 0, sizeof(original_messag));
                      strcpy(messag, strtok(NULL, ""));
                      strcpy(original_messag, messag);

                      char *fr = strtok(original_messag, " ");
                      char *l = strtok(NULL, " ");

                      int length = atoi(l);
                      char *next;
                      next = strtok(NULL, "");
                      memset(&mesg, 0, sizeof(mesg));
                      strncpy(mesg, next, length);
                      cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                      cse4589_print_and_log("msg from:%s\n[msg]:%s\n", fr, mesg);
                      cse4589_print_and_log("[%s:END]\n", "RECEIVED");

                      list_msg[0] = strtok(messag, " ");
                      if (strcmp(list_msg[0], "BUFFER") == 0 || list_msg[0] == NULL)
                      {
                        continue;
                      }

                      while ((list_msg[0] = strtok(NULL, " ")) != NULL && strcmp(list_msg[0], "BUFFER") != 0)
                        continue;
                    }
                    if (list_msg[0] == NULL)
                    {
                      cse4589_print_and_log("[LOGIN:SUCCESS]\n");
                      cse4589_print_and_log("[LOGIN:END]\n");
                      break;
                    }

                    for (int j = 1; j != 3; ++j)
                    {
                      memset(&list_msg[j], 0, sizeof(list_msg[j]));
                      list_msg[j] = strtok(NULL, " ");
                    }
                    struct socket_info si;
                    strcpy(si.host_name, list_msg[0]);
                    strcpy(si.ip_address, list_msg[1]);
                    int port_n = atoi(list_msg[2]);
                    si.port_number = port_n;
                    strcpy(si.status, "logged-in");
                    info_struct.clients.push_back(si);
                  }
                }
                else if (strcmp(arg_zero, "REFRESH") == 0)
                {
                  info_struct.clients.clear();
                  while (true)
                  {
                    char *list_msg[3];
                    if ((list_msg[0] = strtok(NULL, " ")) == NULL)
                      break;

                    for (int j = 1; j != 3; ++j)
                    {
                      memset(&list_msg[j], 0, sizeof(list_msg[j]));
                      list_msg[j] = strtok(NULL, " ");
                    }
                    struct socket_info si;
                    strcpy(si.host_name, list_msg[0]);
                    strcpy(si.ip_address, list_msg[1]);
                    int port_n = atoi(list_msg[2]);
                    si.port_number = port_n;
                    strcpy(si.status, "logged-in");
                    info_struct.clients.push_back(si);
                  }
                }
              }
            }
          }
        }
        freeaddrinfo(result);
      }
    }

    else if (strcmp(user_input, "EXIT") == 0)
    {
      cse4589_print_and_log("[EXIT:SUCCESS]\n");
      cse4589_print_and_log("[EXIT:END]\n");
      break;
    }
  }
}