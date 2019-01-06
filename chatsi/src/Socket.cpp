#include "../include/Socket.hpp"

sockaddr_in make_ip_address(const std::string& ip, int port) {
  sockaddr_in local_address{};

  local_address.sin_family = AF_INET;
  if (ip.empty()) {
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    int e_inet_aton = inet_aton(ip.data(),&local_address.sin_addr);
    if (e_inet_aton == 0) {
      throw std::invalid_argument("La dirección indicada es inválida.");
    }
  }
  local_address.sin_port = htons(port);

  return local_address;
}

//AF_INET porque queremos conectarnos a internet; SOCK_DGRAM porque no queremos solicitar conexion, sino mandarsela porque si (UDP)
Socket::Socket (const sockaddr_in& addr) {
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ < 0) {
    //std::cerr << "Chatsi: no se pudo conectar el socket: " << std::strerror(errno) << '\n';
    throw std::system_error(errno, std::system_category(), "no se pudo crear el socket.");
  }

  int result = bind(fd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
  if (result < 0) {
    //std::cerr << "Chatsi: no se pudo asignar la dirección al socket: " << std::strerror(errno) << "\n";
    throw std::system_error(errno, std::system_category(), "no se pudo establecer conexión con el socket.");
  }
}

Socket::~Socket (void) {
  close(fd_);
}

void Socket::send_to(const Message& message, const sockaddr_in& address) {
  int result = sendto(fd_, &message, sizeof(message), 0,
    reinterpret_cast<const sockaddr*>(&address), sizeof(address));
  if (result < 0) {
    //std::cerr << "Falló sendto: " << std::strerror(errno) << "\n";
    throw std::system_error(errno, std::system_category(), "no se pudo enviar el mensaje.");
  }
}

Message Socket::receive_from(sockaddr_in& address) {
  Message message;
  socklen_t src_len = sizeof(address);
  int result = recvfrom(fd_, &message, sizeof(message), 0,
  reinterpret_cast<sockaddr*>(&address), &src_len);
  if (result < 0) {
    //std::cerr << "Falló recvfrom: " << std::strerror(errno) << "\n";
    throw std::system_error(errno, std::system_category());
  }
  return message;
}
