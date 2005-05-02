#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef NULL
  #define NULL ((void*)0)
#endif

#define PUBLISHED(name,fn) int fn(int argc, char** argv)
#define ERR_PARAMCOUNT 1
#define ERR_PARAMPARSE 2
#define ERR_EXEC	   3

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
