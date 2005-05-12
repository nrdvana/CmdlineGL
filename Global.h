#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef NULL
  #define NULL ((void*)0)
#endif

#ifndef bool
  #define bool int
#endif
#ifndef true
  #define true 1
#endif
#ifndef false
  #define false 0
#endif

#define MAX_COMMAND_BATCH 8

#define MAX_GL_PARAMS 32

#define PUBLISHED(name,fn) int fn(int argc, char** argv)
#define ERR_PARAMCOUNT 1
#define ERR_PARAMPARSE 2
#define ERR_EXEC	   3

//#define DEBUG

#ifdef DEBUG
  #define DEBUGMSG(a) DebugMsg a
  void DebugMsg(char *str, ...);
#else
  #define DEBUGMSG(a) do{}while(0)
#endif

extern bool IsGlBegun; // indicates status of glBegin/glEnd

#endif
