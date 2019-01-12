#pragma once

#include <iostream>
#include <dirent.h>   //Para opendir()
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "Socket.hpp"

#define HISTORY_SIZE 1048576

class History {

private:
  int fd_;
  void* mmapArea_;
  int pointer_;
public:
  History(std::string username);
  ~History(void);

  void add_message (Message message);
};
