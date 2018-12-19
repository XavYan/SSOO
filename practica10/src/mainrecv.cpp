#include "../include/Socket.hpp"

int main (void) {

  sockaddr_in local_address = make_ip_address("127.0.0.1", 3000);
  sockaddr_in dest_address = make_ip_address("127.0.0.2", 2000);
  Socket socket(local_address); //Creando el socket local

  std::string message;
  std::string line;

  while (message != "/quit" && line != "/quit") {

    std::cout << "Esperando a por el mensaje...\n";
    message = socket.receive_from(dest_address);
    if (message != "/quit") {
      std::cout << inet_ntoa(dest_address.sin_addr) << ": " << message << "\n";

      std::cout << "Mensaje > ";
      std::getline(std::cin, line);
      socket.send_to(line, dest_address);
    }
  }
  return 0;
}
