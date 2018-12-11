#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

sockaddr_in make_ip_address(const std::string& ip, int port);

class Socket {
private:
  int fd_;

public:
  Socket(const sockaddr& addr);
  ~Socket();

  void send_to(const std::string& message /* address */ );
  std::string receive_from(/* addres */);
};
