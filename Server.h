#ifndef SERVER_H
#define SERVER_H

#include "Global.h"

PUBLISHED(cglQuit,DoQuit);
PUBLISHED(cglExit,DoQuit);
PUBLISHED(cglGetTime,DoGetTime);
PUBLISHED(cglSync,DoSync);
PUBLISHED(cglSleep, DoSleep);
PUBLISHED(cglEcho,DoEcho);

extern bool PendingResize;
extern void FinishResize();

#endif
