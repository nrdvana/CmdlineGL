#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef NULL
  #define NULL ((void*)0)
#endif

#define PUBLISHED(name,fn) void fn(int argc, char** argv)

#ifndef bool
  #define bool int
#endif
#ifndef true
  #define true 1
#endif
#ifndef false
  #define false 0
#endif

//#define DEBUG

#ifdef DEBUG
  #define DEBUGMSG DebugMsg
  void DebugMsg(char *str, ...);
#else
  #define DEBUGMSG
#endif


#endif
