#include "../include/Socket.hpp"

int main (void) {

  sockaddr_in local_address = make_ip_address("127.0.0.2", 2000);
  sockaddr_in dest_address = make_ip_address("127.0.0.1", 3000);
  Socket socket(local_address); //Creando el socket local

  std::string message;
  std::string line;

  while (message != "/quit" && line != "/quit") {

    std::cout << "Mensaje > ";
    std::getline(std::cin, line);
    socket.send_to(line, dest_address);

    if (line != "/quit") {
      std::cout << "Esperando a por el mensaje del otro usuario...\n";
      message = socket.receive_from(dest_address);
      if (message != "/quit") {
        std::cout << inet_ntoa(dest_address.sin_addr.s_addr) << ": " << message << "\n";
      } else {
        std::cout << "El usuario " << inet_ntoa(dest_addres.sin_addr.s_addr) << " ha cerrado la sesión.\n";
      }
    }
  }

  return 0;
}
