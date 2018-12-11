#include "../include/Socket.hpp"

in main (void) {

  sockaddr_in local_address = make_ip_address("12.0.0.1", 2000);
  sockaddr_in dest_address = make_ip_address("12.0.0.1", 3000);
  Socket socket(local_address);

  //Repetir

  std::string line;
  std::getline(std::cin, line);

  socket.send_to(line /* "127.0.0.1" " 3000 " */);

  std::string message = socket.receive_from();
  std::cout << message;

  //Repetir
}
