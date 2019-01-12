#include "../include/history.hpp"

History::History(std::string username) : fd_(0), mmapArea_(), pointer_(0) {
  //Si el directorio no existe se debe crear
  DIR* dir = opendir("~/.chatsi");
  if (dir) { //El directorio existe
    closedir(dir);
  } else if (ENOENT == errno) { //El directorio no existe
    mkdir("~/.chatsi");
  }
  fd_ = open ("~/.chatsi/"+username+".log", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd_ < 0) {
    throw std::system_error(errno, std::system_category(), "no se pudo abrir el archivo del historial.\n");
  }
  //Truncamos el archivo con ftruncate()
  ftruncate(fd, HISTORY_SIZE);

  //Mapeamos en memorira con mmap()
  int error_lockf = lockf(fd_, F_TLOCK, 0); //Bloqueamos el archivo
  if (error_lockf < 0) {
    if (errno == EACCES || errno == EAGAIN) throw std::system_error(errno, std::system_category(), "El nombre de usuario ya se esta usando en este equipo.\n");
    throw std::system_error(errno, std::system_category(), "Fallo al bloquear el archivo.\n");
  }
  mmapArea_ = mmap(NULL, HISTORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (mmapArea_ == MAP_FAILED) {
    throw std::system_error(errno, std::system_category(), "Fallo al mapear en memoria.\n");
  }
}

History::~History (void) {
  int error_mmap = munmap(mmapArea_, HISTORY_SIZE);
  if (error_mmap < 0) {
    throw std::system_error(errno, std::system_category(), "Error al desmapear el archivo.\n");
  }
  int error_lockf = lockf(fd_, F_ULOCK, 0);
  if (error_lockf < 0) {
    throw std::system_error(errno, std::system_category(), "Error al desbloquear el archivo.\n");
  }
  close(fd_);
}

void History::add_message (Message message) {
  std::string line = message.username + " dijo: " + message.text "\n";
  int num_byte =  line.copy(pointer_, line.length(), 0);
  pointer_ += num_byte;
}
