#include <thread>
#include <pthread.h>
#include <csignal>
#include <cxxabi.h>
#include <atomic>
#include <set>
#include <getopt.h>
#include <cstdlib>    // para std::getenv() y std::setenv()
#include "../include/Socket.hpp"

//Declaracion de variables
bool help_mode = false;
bool server_mode = false;
bool client_mode = false;
std::string username = std::getenv("USER");
std::string line;
sockaddr_in local_address;
sockaddr_in dest_address;
std::set<std::pair<uint32_t, in_port_t> > destination_adresses;

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
  //Bloqueamos las señales SIGTERM, SIGINT y SIGHUP
  sigset_t set;
  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGHUP);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  try {
    while(line != "/quit") {
      Message message;
      std::getline(std::cin, line);
      if (line != "/quit") {
        strcpy(message.text, line.c_str());
        strcpy(message.username, username.c_str());
        message.ip = local_address.sin_addr.s_addr;
        message.port = local_address.sin_port;
        message.with_name = 1;
        std::cout << "\x1b[1A\x1b[2K";
        if (server_mode) {
          for (std::pair<uint32_t, in_port_t> user : destination_adresses) {
            sockaddr_in address {};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = std::get<0>(user);
            address.sin_port = std::get<1>(user);
            socket.send_to(message, address);
          }
          std::cout << username << ": " << message.text << "\n";
        } else if (client_mode) {
          socket.send_to(message, dest_address);
        } else {
          std::cout << "Chatsi: Ha ocurrido un error al enviar el mensaje.\n";
          break;
        }
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
  //Bloqueamos las señales SIGTERM, SIGINT y SIGHUP
  sigset_t set;
  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGHUP);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  Message message {};

  try {
    while (true) {
      message = socket.receive_from(dest_address);
      if (server_mode) { //Si estas en modo servidor, aquellos que envien por primera vez el mensaje se deben insertar en el conjunto
        std::pair<uint32_t, in_port_t> user;
        std::get<0>(user) = message.ip;
        std::get<1>(user) = message.port;
        if ((message.ip != local_address.sin_addr.s_addr) && (destination_adresses.find(user) == destination_adresses.end())) {
          destination_adresses.insert(user);
        }
        //Enviamos el mensaje a cada usuario
        for (std::pair<uint32_t, in_port_t> user : destination_adresses) {
          sockaddr_in address {};
          address.sin_family = AF_INET;
          address.sin_addr.s_addr = std::get<0>(user);
          address.sin_port = std::get<1>(user);
          socket.send_to(message, address);
        }
      }
      if (message.with_name == 1) {
        std::cout << message.username << ": " << message.text << "\n";
      } else {
        std::cout << message.text << "\n";
      }
    }
  } catch (abi::__forced_unwind&) {
    throw;
  } catch (...) {
    eptr = std::current_exception();
  }
  quit = true;
}

void usage (std::ostream& os) {
  os << "Usage: [-h/--help] [-c/--client] [-s/--server] [-p/--port]\n";
}

int main (int argc, char* argv[]) {

  //Manejando señales
  std::signal(SIGINT, &int_signal_handler); //Señal al pulsar Ctrl-C
  std::signal(SIGTERM, &int_signal_handler); //Señal al cerrar la terminal
  std::signal(SIGHUP, &int_signal_handler); //Señal al apagar PC

  try {
    std::exception_ptr eptr1 {};
    std::exception_ptr eptr2 {};

    int port_option = -1;
    std::string client_option;

    //Declaramos la estructura de long_option
    static struct option long_options[] = {
      {"help",      no_argument,        0,  'h'},
      {"server",    no_argument,        0,  's'},
      {"client",    required_argument,  0,  'c'},
      {"port",      required_argument,  0,  'p'},
      {"username",  required_argument,  0,  'u'},
      {0, 0, 0, 0}
    };

    int c; //Caracter que se va leyendo a traves de las opciones indicadas en argv
    while ((c = getopt_long(argc, argv, "hsc:p:u:", long_options, nullptr)) != -1) {
      switch (c) {
        case 'h': //Mostrar ayuda INCOMPATIBLE CON LOS DEMAS ARGUMENTOS
          help_mode = true;
          break;
        case 's': //Modo servidor INCOMPATIBLE CON MODO CLIENTE ('c') Y CON AYUDA ('h'). SI NO SE ESPECIFICA PUERTO ('p'), SE LE ASIGNA UNO ALEATORIO (?)
          server_mode = true;
          break;
        case 'c': //Modo cliente INCOMPATIBLE CON MODO SERVIDOR ('s') Y CON AYUDA ('h'). NECESARIO PUERTO ('p') Y ARGUMENTO.
          client_option = std::string(optarg);
          if (client_option.empty()) {
            std::cerr << "La opcion '-c' necesita un argumento. Use \"./chatsi -h\" o \"./chatsi --help\" para conocer mas acerca del funcionamiento del programa.\n";
            usage(std::cerr);
            return 1;
          }
          client_mode = true;
          break;
        case 'p': //Indica el puerto NECESARIO ARGUMENTO. INCOMPATIBLE CON AYUDA ('h'). SI NO SE INDICA MODO, POR DEFECTO MODO SERVIDOR.
          port_option = stoi(std::string(optarg));
          break;
        case 'u':
          username = std::string(optarg);
          break;
        case '?':
          // No hacemos nada porque optarg se encarga de mostrar un mensaje de error
          return 1;
          break;
        default:
          std::cerr << "?? La funcion getopt devolvio codigo de error " << c << '\n';
          return 1;
      }
    }

    if (help_mode) {
      usage(std::cout);
      std::cout << "  -h / --help:\n";
      std::cout << "\tMuestra la ayuda de uso, tal y como ahora esta.\n";
      std::cout << "  -c / --client:\n";
      std::cout << "\tInicia el programa en modo cliente. Es necesario indicar como argumento la IP del servidor.\n\tSi no se especifica puerto de escucha (con la opcion 'p') se usara por defecto el puerto 8000\n";
      std::cout << "  -s / --server:\n";
      std::cout << "\tInicia el programa en modo servidor. Si no se especifica puerto con la opcion '-p' un puerto, el programa le asignara un puerto libre.\n";
      std::cout << "  -p / --port:\n";
      std::cout << "\tCon este comando podemos indicar un puerto de conexion.\n\tSi no se especifica ninguna opcion de modo ('-s' o '-c') el programa por defecto iniciara en modo servidor.\n";
      return 0;
    }

    if (server_mode) {
      if (client_mode) {
        std::cerr << "El modo cliente ('-c') y el modo servidor ('-s') no pueden ser usados al mismo tiempo. Use \"./chatsi -h\" o \"./chatsi --help\" para conocer mas acerca del funcionamiento del programa.\n";
        usage(std::cerr);
        return 2;
      }
      if (port_option < 0) { //Si no especifica puerto con '-p' se entiende que el puerto se lo asigna el programa
        port_option = 0;
      }
      local_address = make_ip_address("", port_option);
      dest_address = local_address;
    }

    if (client_mode) {
      if (port_option < 0) { //Le asignamos el puerto 8000 si el usuario no ha asignado ningun puerto
        port_option = 8000;
      }
      local_address = make_ip_address("", 0);
      dest_address = make_ip_address(client_option, port_option);
    }


    Socket socket(local_address); //Creando el socket local

    if (client_mode) {
      Message client_connection{};
      client_connection.with_name = 0;
      client_connection.ip = local_address.sin_addr.s_addr;
      client_connection.port = local_address.sin_port;
      strcpy(client_connection.text, std::string(username + " se ha conectado a la sesion.\n").c_str());
      socket.send_to(client_connection, dest_address); //Mandamos un mensaje para avisar de que se ha conectado un usuario
    }

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
