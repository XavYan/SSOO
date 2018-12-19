#include <thread>
#include "../include/Socket.hpp"

/**
 * CREAR CLASE USUARIO
 */

std::string line;
std::string message;
sockaddr_in local_address;
sockaddr_in dest_address;

void thread_send (Socket& socket) {
  while(line != "/quit") {
    std::getline(std::cin, line);
    if (line != "/quit") {
      std::cout << inet_ntoa(local_address.sin_addr) << ": " << line << "\n";
      socket.send_to(line,dest_address);
    }
    else std::cout << "Has salido de la sesión.\n";
  }
}

void thread_recv (Socket& socket) {
  while (line != "/quit") {
    message = socket.receive_from(dest_address);
    /*Prototipo para mejorar la impresion del mensaje recibido */
    //E imprimimos el mensaje
    if (message != "/quit") std::cout << inet_ntoa(dest_address.sin_addr) << ": " << message << "\n";
    else std::cout << "El usuario " << inet_ntoa(dest_address.sin_addr) << " ha salido de la sesión.\n";
  }
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

    local_address = make_ip_address("127.0.0.1", port1);
    dest_address = make_ip_address("127.0.0.1", port2);


    Socket socket(local_address); //Creando el socket local



      std::thread send (&thread_send,std::ref(socket));
      std::thread recv (&thread_recv,std::ref(socket));

      send.join(); //Acabar con todos los hilos cuando line == /quit
      if (line != "/quit") recv.join();
  } //EXCEPCIONES
  catch (std::system_error& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
  }
  catch(std::invalid_argument& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
  }
  return 0;
}
