#include <thread>
#include <pthread.h>
#include <atomic>
#include "../include/Socket.hpp"

/**
 * CREAR CLASE USUARIO
 */

std::string line;
std::string message;
sockaddr_in local_address;
sockaddr_in dest_address;

std::atomic<bool>& quit (false);

void request_cancellation (std::thread& thread) {
  int err = pthread_cancel(thread.native_handler());
  if (err != 0) {
    throw std::system_error(err, std::system_category(), "Problema al cancelar el hilo.");
  }
}

void thread_send (Socket& socket, std::exception_ptr& eptr) {
  try {
    while(line != "/quit") {
      std::getline(std::cin, line);
      if (line != "/quit") {
        std::cout << inet_ntoa(local_address.sin_addr) << ": " << line << "\n";
        socket.send_to(line,dest_address);
      }
      else std::cout << "Has salido de la sesión.\n";
    }
  } catch (...) {
    eptr = std::current_exception();
  }
  quit = true;
}

void thread_recv (Socket& socket, std::exception_ptr& eptr) {
  try {
    while (line != "/quit") {
      message = socket.receive_from(dest_address);
      if (message != "/quit") std::cout << inet_ntoa(dest_address.sin_addr) << ": " << message << "\n";
      else std::cout << "El usuario " << inet_ntoa(dest_address.sin_addr) << " ha salido de la sesión.\n";
    }
  } catch (...) {
    eptr = std::current_exception();
  }
  quit = true;
}

int main (void) {
  try {
    std::exception_ptr eptr1 {};
    std::exception_ptr eptr2 {};
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



      std::thread send (&thread_send,std::ref(socket), std::ref(eptr1));
      std::thread recv (&thread_recv,std::ref(socket), std::ref(eptr2));

      while(!quit); //Espera a que algun hilo termine

      //Terminamos todos los hilos
      request_cancellation(send);
      request_cancellation(recv);

      send.join(); //Acabar con todos los hilos cuando line == '/quit'
      if (line != "/quit") recv.join();

      if (eptr1) {
        std::rethrow_exception(eptr1);
      }
      if (eptr2) {
        std::rethrow_exception(eptr2);
      }
  } //EXCEPCIONES
  catch (std::bad_alloc& e) {
    std::cerr << "chatsi: memoria insuficiente.\n";
    return 1;
  }
  catch (std::system_error& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
    return 2;
  }
  catch(std::invalid_argument& e) {
    std::cerr << "Chatsi: " << e.what() << "\n";
    return 3;
  }
  return 0;
}
