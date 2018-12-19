#include <thread>
#include "../include/Socket.hpp"

std::string line;
std::string message;

void thread_send (Socket& socket, sockaddr_in& dest_address) {
  std::getline(std::cin, line);
  socket.send_to(line,dest_address);
}

void thread_recv (Socket& socket, sockaddr_in& dest_address) {
  message = socket.receive_from(dest_address);
  if (message != "/quit") std::cout << inet_ntoa(dest_address.sin_addr) << ": " << message << "\n";
  else std::cout << "El usuario " << inet_ntoa(dest_address.sin_addr) << " ha salido de la sesión.\n";
}

int main (void) {
  try {
    std::cout << "Conexion a nivel interno.\n";

    //Declaracion de variables
    int port1,port2;

    std::cout << "Puerto a conectar: ";
    std::cin >> port1;
    std::cout << "Puerto de escucha: ";
    std::cin >> port2;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    sockaddr_in local_address = make_ip_address("127.0.0.1", port1);
    sockaddr_in dest_address = make_ip_address("127.0.0.1", port2);

    Socket socket(local_address); //Creando el socket local

    while (line != "/quit") {

      std::thread send (&thread_send,std::ref(socket),std::ref(dest_address));
      std::thread recv (&thread_recv,std::ref(socket),std::ref(dest_address));

      send.join();
      if (line != "/quit") recv.join();
    }
  } //EXCEPCIONES
  catch (std::system_error& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
  }
  catch(std::invalid_argument& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
  }
  return 0;
}
