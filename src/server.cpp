#include "../include/server.h"
#include "../include/logger.h"
#include "../include/structures.h"

#include <algorithm>
#include <arpa/inet.h>

#include <list>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#include <unistd.h>

#define STD_IN 0
using namespace std;

void output_error_log(const char *command_str)
{
  cse4589_print_and_log("[%s:ERROR]\n", command_str);
  cse4589_print_and_log("[%s:END]\n", command_str);
}

bool validate_ip_address(char *server_ip_address, int *p)
{
  struct sockaddr_in ip_address;

  ip_address.sin_family = AF_INET;

  if (server_ip_address == NULL)
  {
    return false;
  }

  if (p != NULL)
  {
    ip_address.sin_port = htons(*p);
  }

  if (inet_pton(AF_INET, server_ip_address, &ip_address.sin_addr) != 1)
  {
    return false;
  }

  return true;
}

server::server(char *port)
{

  strcpy(info_struct.port_number, port);

  struct hostent *host_ent;
  char hostname[1024];
  if (gethostname(hostname, 1024) < 0)
  {
    cerr << "unable to fetch host name" << endl;
    exit(1);
  }
  if ((host_ent = gethostbyname(hostname)) == NULL)
  {
    cerr << "host name is null" << endl;
    exit(1);
  }

  struct in_addr **addr_list = (struct in_addr **)host_ent->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; ++i)
  {
    strcpy(info_struct.ip_address, inet_ntoa(*addr_list[i]));
  }

  if ((info_struct.listener_number = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    cerr << "error making socket\n";
    exit(1);
  }

  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(atoi(port));
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (::bind(info_struct.listener_number, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
  {
    cerr << "error binding socket\n";
    exit(1);
  }

  if (listen(info_struct.listener_number, 8) < 0)
  {
    cerr << "error listening\n";
    exit(1);
  }

  fd_set master_fd_set;
  fd_set read_fds;

  FD_ZERO(&master_fd_set);
  FD_ZERO(&read_fds);

  FD_SET(info_struct.listener_number, &master_fd_set);
  FD_SET(STD_IN, &master_fd_set);

  int fd_max = info_struct.listener_number;
  int address_len;

  int received_bytes;

  char user_input[1024];
  struct sockaddr_in remote_address;

  while (true)
  {
    read_fds = master_fd_set;

    if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) < 0)
    {
      cerr << "Error selecting\n";
      exit(1);
    }

    for (int i = 0; i <= fd_max; i++)
    {
      memset((void *)&user_input, '\0', sizeof(user_input));
      if (FD_ISSET(i, &read_fds))
      {
        if (i == STD_IN)
        {

          read(STD_IN, user_input, 1024);
          user_input[strlen(user_input) - 1] = '\0';

          if (strcmp(user_input, "AUTHOR") == 0)
          {
            cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
            cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "pa1-nsaji-atembulk");
            cse4589_print_and_log("[AUTHOR:END]\n");
          }

          else if (strcmp(user_input, "PORT") == 0)
          {
            cse4589_print_and_log("[PORT:SUCCESS]\n");
            cse4589_print_and_log("PORT:%s\n", info_struct.port_number);
            cse4589_print_and_log("[PORT:END]\n");
          }

          else if (strcmp(user_input, "IP") == 0)
          {
            cse4589_print_and_log("[IP:SUCCESS]\n");
            cse4589_print_and_log("IP:%s\n", info_struct.ip_address);
            cse4589_print_and_log("[IP:END]\n");
          }

          else if (strcmp(user_input, "STATISTICS") == 0)
          {

            info_struct.clients.sort([](const socket_info &si1, const socket_info &si2)
                                     { return si1.port_number < si2.port_number; });

            cse4589_print_and_log("[STATISTICS:SUCCESS]\n");

            int index = 0;
            for (const auto &client : info_struct.clients)
            {
              cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", ++index, client.host_name,
                                    client.message_count_sent, client.received_message_count, client.status);
            }

            cse4589_print_and_log("[STATISTICS:END]\n");
          }

          else if (strcmp(user_input, "LIST") == 0)
          {

            info_struct.clients.sort([](const socket_info &si1, const socket_info &si2)
                                     { return si1.port_number < si2.port_number; });

            cse4589_print_and_log("[LIST:SUCCESS]\n");

            int index = 0;
            for (const auto &client : info_struct.clients)
            {
              if (strcmp(client.status, "logged-in") == 0)
              {
                cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", ++index, client.host_name,
                                      client.ip_address, client.port_number);
              }
            }

            cse4589_print_and_log("[LIST:END]\n");
          }

          else if (strncmp(user_input, "BLOCKED", 7) == 0)
          {
            char *arg[2];
            arg[0] = strtok(user_input, " ");
            arg[1] = strtok(NULL, " ");

            if (!validate_ip_address(arg[1], NULL))
            {
              output_error_log("BLOCKED");
            }

            else
            {
              bool valid = false;
              for (auto &iter : info_struct.clients)
              {
                {
                  if (strcmp(iter.ip_address, arg[1]) == 0)
                  {

                    iter.blocked_clients_list.sort([](const blocked_clients &b1, const blocked_clients &b2)
                                                   { return b1.listen_port_num < b2.listen_port_num; });

                    cse4589_print_and_log("[BLOCKED:SUCCESS]\n");

                    int index = 0;
                    for (const auto &blocked_client : iter.blocked_clients_list)
                    {
                      cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", ++index, blocked_client.host_name,
                                            blocked_client.ip_address, blocked_client.listen_port_num);
                    }

                    cse4589_print_and_log("[BLOCKED:END]\n");
                    valid = true;
                    break;
                  }
                }
              }

              if (!valid)
              {
                output_error_log("BLOCKED");
              }
            }
          }
        }
        else if (i == info_struct.listener_number)
        {
          int client_file_descriptor;
          address_len = sizeof(remote_address);
          if ((client_file_descriptor = accept(info_struct.listener_number, (struct sockaddr *)&remote_address, (socklen_t *)&address_len)) == -1)
          {
            cerr << "error accepting new connection" << endl;
          }
          else
          {
            FD_SET(client_file_descriptor, &master_fd_set);
            if (client_file_descriptor > fd_max)
            {
              fd_max = client_file_descriptor;
            }

            bool flag = true;

            struct sockaddr_in *sin = (struct sockaddr_in *)&remote_address;
            char *received_ip = inet_ntoa(sin->sin_addr);

            for (auto &client : info_struct.clients)
            {
              if (strcmp(received_ip, client.ip_address) == 0)
              {
                strcpy(client.status, "logged-in");
                client.fd = client_file_descriptor;
                flag = false;
                break;
              }
            }

            if (flag)
            {
              const char *sts = "logged-in";
              struct socket_info si;
              si.list_id = info_struct.clients.size() + 1;

              struct in_addr *addr_temp;
              struct sockaddr_in saddr;
              struct hostent *host_ent;

              if (!inet_aton(received_ip, &saddr.sin_addr))
              {
                cerr << "error validating IP address" << endl;
                exit(1);
              }

              if((host_ent = gethostbyaddr((const void *)&saddr.sin_addr,sizeof(received_ip),AF_INET)) == NULL){
                cerr<<"error getting host by address"<<endl;
                exit(1);
              }
              char *n = host_ent->h_name;
              
              strcpy(si.host_name, n);

              strcpy(si.ip_address, received_ip);
              si.fd = client_file_descriptor;
              strncpy(si.status, sts, strlen(sts));

              char client_port[8];
              memset(&client_port, 0, sizeof(client_port));
              if (recv(client_file_descriptor, client_port, sizeof(client_port), 0) <= 0)
              {
                cerr << "error receiving message" << endl;
              }
              si.port_number = atoi(client_port);

              info_struct.clients.push_back(si);
            }

            char notification_payload[4096];

            memset(&notification_payload, 0, sizeof(notification_payload));
            strcat(notification_payload, "LOGIN ");

            for (const auto &client : info_struct.clients)
            {
              if (strcmp(client.status, "logged-in") == 0)
              {
                snprintf(notification_payload + strlen(notification_payload), sizeof(notification_payload) - strlen(notification_payload),
                         "%s %s %d ", client.host_name, client.ip_address, client.port_number);
              }
            }

            for (auto &client : info_struct.clients)
            {
              if (strcmp(client.ip_address, received_ip) == 0)
              {
                while (!client.buffer.empty())
                {
                  buffer_info stored_messages = client.buffer.front();
                  snprintf(notification_payload + strlen(notification_payload), sizeof(notification_payload) - strlen(notification_payload),
                           "BUFFER %s %d %s ", stored_messages.fr, (int)strlen(stored_messages.message), stored_messages.message);
                  client.buffer.pop();

                  cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
                  cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", stored_messages.fr, stored_messages.destination_ip_address, stored_messages.message);
                  cse4589_print_and_log("[%s:END]\n", "RELAYED");
                }
              }
            }

            if (send(client_file_descriptor, notification_payload, strlen(notification_payload), 0) < 0)
            {
              cerr << "error sending messages to client" << endl;
            }
          }
        }

        else
        {
          char message_from_client[1024];
          memset(&message_from_client, 0, sizeof(message_from_client));

          if ((received_bytes = recv(i, message_from_client, sizeof(message_from_client), 0)) <= 0)
          {

            if (received_bytes == 0)
            {

              info_struct.num_clients--;
              for (list<socket_info>::iterator iter = info_struct.clients.begin(); iter != info_struct.clients.end(); ++iter)
              {
                if (iter->fd == i)
                {
                  const char *stso = "logged-out";
                  strncpy(iter->status, stso, strlen(stso));
                }
              }
            }
            else
            {
              cerr << "error receiving message from client" << endl;
            }
            close(i);
            FD_CLR(i, &master_fd_set);
          }
          else
          {

            char original_msg[1024];
            char buffer_msg[1024];

            memset(&original_msg, 0, sizeof(original_msg));
            memset(&buffer_msg, 0, sizeof(buffer_msg));

            strcpy(original_msg, message_from_client);
            strcpy(buffer_msg, message_from_client);
            char *arg_zero = strtok(message_from_client, " ");

            if (strcmp(arg_zero, "SEND") == 0)
            {
              char from_client_ip[32] = {0};
              for (auto &client : info_struct.clients)
              {
                if (client.fd == i)
                {
                  strcpy(from_client_ip, client.ip_address);
                  client.message_count_sent++;
                  break;
                }
              }

              char *arg[3];
              arg[1] = strtok(NULL, " ");
              arg[2] = strtok(NULL, "");

              char new_msg[1024];
              snprintf(new_msg, sizeof(new_msg), "SEND %s %s %s", from_client_ip, arg[1], arg[2]);

              bool blocked = false;
              bool login_status = true;

              for (auto &client : info_struct.clients)
              {
                if (strcmp(client.ip_address, arg[1]) == 0)
                {
                  if (strcmp(client.status, "logged-out") == 0)
                  {
                    login_status = false;
                  }
                  for (const auto &blocked_client : client.blocked_clients_list)
                  {
                    if (strcmp(blocked_client.ip_address, from_client_ip) == 0)
                    {
                      blocked = true;
                      break;
                    }
                  }
                }
              }

              if (login_status && !blocked)
              {
                cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
                cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from_client_ip, arg[1], arg[2]);
                cse4589_print_and_log("[%s:END]\n", "RELAYED");

                for (auto &client : info_struct.clients)
                {
                  if (strcmp(client.ip_address, arg[1]) == 0)
                  {
                    if (send(client.fd, new_msg, strlen(new_msg), 0) < 0)
                    {
                      cerr << "send" << endl;
                    }
                    client.received_message_count++;
                    break;
                  }
                }

                memset(&message_from_client, 0, sizeof(message_from_client));
              }

              if (!login_status && !blocked)
              {

                buffer_info buffer_data;
                strcpy(buffer_data.destination_ip_address, arg[1]);
                strcpy(buffer_data.message, arg[2]);
                strcpy(buffer_data.fr, from_client_ip);

                for (auto &client : info_struct.clients)
                {
                  if (strcmp(client.ip_address, arg[1]) == 0)
                  {
                    client.buffer.push(buffer_data);
                    client.received_message_count++;
                  }
                }
              }
            }

            else if (strcmp(arg_zero, "REFRESH") == 0)
            {
              char client_message_buffer[4096] = {0};
              char *received_ip = strtok(NULL, " ");

              snprintf(client_message_buffer, sizeof(client_message_buffer), "REFRESH ");

              for (const auto &client : info_struct.clients)
              {
                if (strcmp(client.status, "logged-in") == 0)
                {
                  snprintf(client_message_buffer + strlen(client_message_buffer),
                           sizeof(client_message_buffer) - strlen(client_message_buffer),
                           "%s %s %d ", client.host_name, client.ip_address, client.port_number);
                }
              }

              if (send(i, client_message_buffer, strlen(client_message_buffer), 0) < 0)
              {
                cerr << "error sending REFRESH event" << endl;
              }
            }

            else if (strcmp(arg_zero, "BROADCAST") == 0)
            {
              cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
              char from_client_ip[32] = {0};

              for (auto &client : info_struct.clients)
              {
                if (client.fd == i)
                {
                  strncpy(from_client_ip, client.ip_address, sizeof(from_client_ip) - 1);
                  client.message_count_sent++;
                  break;
                }
              }

              char *arg[2];
              arg[1] = strtok(NULL, "");

              char new_msg[1024] = {0};

              snprintf(new_msg, sizeof(new_msg), "BROADCAST %s %s", from_client_ip, arg[1]);

              cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n",
                                    from_client_ip, arg[1]);

              cse4589_print_and_log("[%s:END]\n", "RELAYED");

              for (auto &client : info_struct.clients)
              {
                if (client.fd == i)
                {
                  continue;
                }

                bool blocked = false;
                for (const blocked_clients &blocked_client : client.blocked_clients_list)
                {
                    // std::cout << "[DEBUG] Checking against: " << blocked_client.ip_address << std::endl;

                    if (strcmp(blocked_client.ip_address, from_client_ip) == 0)
                    {
                        blocked = true;
                        break;
                    }
                }

                if (strcmp(client.status, "logged-in") == 0 && !blocked)
                {
                  if (send(client.fd, new_msg, strlen(new_msg), 0) < 0)
                  {
                    output_error_log("BROADCAST");
                  }
                  client.received_message_count++;
                }

                if (strcmp(client.status, "logged-out") == 0 && !blocked)
                {
                  buffer_info buffer_data;
                  strcpy(buffer_data.destination_ip_address, client.ip_address);
                  strcpy(buffer_data.message, arg[1]);
                  strcpy(buffer_data.fr, from_client_ip);

                  client.buffer.push(buffer_data);
                  client.received_message_count++;
                }
              }
            }
            else if (strcmp(arg_zero, "BLOCK") == 0)
            {
              char *arg[2];
              arg[1] = strtok(NULL, " ");

              for (auto &client : info_struct.clients)
              {
                if (client.fd == i)
                {
                  auto block_iter = std::find_if(info_struct.clients.begin(), info_struct.clients.end(),
                                                 [arg](const socket_info &block_client)
                                                 {
                                                   return strcmp(block_client.ip_address, arg[1]) == 0;
                                                 });

                  if (block_iter != info_struct.clients.end())
                  {
                    blocked_clients b;
                    b.listen_port_num = block_iter->port_number;
                    strncpy(b.ip_address, arg[1], sizeof(b.ip_address) - 1);
                    strncpy(b.host_name, block_iter->host_name, sizeof(b.host_name) - 1);

                    client.blocked_clients_list.push_back(b);
                  }
                }
              }
            }

            else if (strcmp(arg_zero, "UNBLOCK") == 0)
            {
              char *arg[2] = {nullptr, nullptr};
              arg[1] = strtok(NULL, " ");

              for (auto &client : info_struct.clients)
              {
                if (client.fd == i)
                {
                  client.blocked_clients_list.remove_if([arg](const blocked_clients &blocked_client)
                                                        { return strcmp(blocked_client.ip_address, arg[1]) == 0; });
                }
              }
            }
          }
        }
      }
    }
  }
}