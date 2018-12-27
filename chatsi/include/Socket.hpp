#pragma once

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cstring> //Para std::strerror()

//Librerias para errores
#include <exception>
#include <cerrno> //Para errno
#include <system_error>

sockaddr_in make_ip_address(const std::string& ip, int port);

class Socket {
private:
  int fd_;

public:

  Socket(const sockaddr_in& addr);
  ~Socket();

  void send_to(const std::string& message,const sockaddr_in& address);
  std::string receive_from(sockaddr_in& address);
};
