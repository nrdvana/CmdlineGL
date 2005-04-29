#ifndef SERVER_H
#define SERVER_H

#include "Global.h"

PUBLISHED(quit,DoQuit);
PUBLISHED(exit,DoQuit);

void StartServer(char* Socketname);

#endif
