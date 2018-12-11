#include "../include/Socket.hpp"

sockaddr_in make_ip_address(const std::string& ip, int port) {
  sockaddr_in local_address{};
  /*Seguir implementando*/
}

//AF_INET porque queremos conectarnos a internet; SOCK_DGRAM porque no queremos solicitar conexion, sino mandarsela porque si
Socket::Socket (const sockaddr_in& addr) {
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ < 0) {
    std::cerr << "No se pudo conectar el socket.\n";
    return;
  }

  int result = bind(fd_, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
  if (result < 0) {
    std::cerr << "No se pudo asignar la dirección al socket.\n";
    return;
  }
}

Socket::~Socket (void) {
  close(fd_);
}

void Socket::send_to(const std::string& message, const sockaddr_in& address) {
  int result = sendto(fd_, message.c_str(), message.length(), 0,
    reinterpret_cast<const sockaddr*>(&remote_address), sizeof(address));
  if (result < 0) {
    std::cerr << "falló sendto.\n";
    return;
  }
}

std::string Socket::receive_from(/* addres */);
