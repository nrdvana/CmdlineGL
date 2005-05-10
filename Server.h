#ifndef SERVER_H
#define SERVER_H

#include "Global.h"

PUBLISHED(quit,DoQuit);
PUBLISHED(exit,DoQuit);
PUBLISHED(sync,DoSync);

void StartServer(char* Socketname);

#endif
