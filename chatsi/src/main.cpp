#include <thread>
#include <pthread.h>
#include <csignal>
#include <cxxabi.h>
#include <atomic>
#include "../include/Socket.hpp"

/**
 * CREAR CLASE USUARIO
 */

std::string line;
std::string message;
sockaddr_in local_address;
sockaddr_in dest_address;

std::atomic<bool> quit (false);

void int_signal_handler (int signum) {
  quit = true;
}

void request_cancellation (std::thread& thread) {
    int err = pthread_cancel(thread.native_handle());
    if (err != 0) {
      throw std::system_error(err, std::system_category(), "Problema al cancelar los hilos.");
    }
}

void thread_send (Socket& socket, std::exception_ptr& eptr) {
  //Bloqueamos las señales SIGTERM y SIGINT
  sigset_t set;
  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGHUP);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  try {
    while(line != "/quit") {
      std::getline(std::cin, line);
      if (line != "/quit") {
        std::cout << inet_ntoa(local_address.sin_addr) << ": " << line << "\n";
        socket.send_to(line,dest_address);
      }
    }
  } catch (abi::__forced_unwind&) {
    throw;
  } catch (...) {
    eptr = std::current_exception();
  }
  quit = true;
}

void thread_recv (Socket& socket, std::exception_ptr& eptr) {

  //Bloqueamos las señales SIGTERM y SIGINT
  sigset_t set;
  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGHUP);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  try {
    while (true) {
      message = socket.receive_from(dest_address);
      std::cout << inet_ntoa(dest_address.sin_addr) << ": " << message << "\n";
    }
  } catch (abi::__forced_unwind&) {
    throw;
  } catch (...) {
    eptr = std::current_exception();
  }
  quit = true;
}

int main (void) {

  //Manejando señales
  std::signal(SIGINT, &int_signal_handler); //Señal al pulsar Ctrl-C
  std::signal(SIGTERM, &int_signal_handler); //Señal al cerrar terminal
  std::signal(SIGHUP, &int_signal_handler); //Señal al apagar PC

  try {
    std::exception_ptr eptr1 {};
    std::exception_ptr eptr2 {};
    std::cout << "Conexion a nivel interno.\n";

    //Declaracion de variables
    int port1,port2;
    std::string ip1,ip2;

    std::cout << "Indica tu IP: ";
    std::cin >> ip1;
    std::cout << "Indica la IP a la que te vas a conectar: ";
    std::cin >> ip2;

    std::cout << "Puerto a conectar: ";
    std::cin >> port1;
    std::cout << "Puerto de escucha: ";
    std::cin >> port2;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    local_address = make_ip_address(ip1, port1);
    dest_address = make_ip_address(ip2, port2);

    Socket socket(local_address); //Creando el socket local

    std::thread send (&thread_send,std::ref(socket), std::ref(eptr1));
    std::thread recv (&thread_recv,std::ref(socket), std::ref(eptr2));

    while(!quit); //Esperar a que algun hilo termine

    //Terminamos todos los hilos
    request_cancellation(send);
    request_cancellation(recv);

    send.join();
    recv.join();


    if (eptr1) {
      std::rethrow_exception(eptr1);
    }
    if (eptr2) {
      std::rethrow_exception(eptr2);
    }
  } //EXCEPCIONES
  catch (std::bad_alloc& e) {
    std::cerr << "Chatsi: memoria insuficiente.\n";
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
  catch(...) {
    std::cerr << "Chatsi: error desconocido.\n";
    return -1;
  }
  return 0;
}
