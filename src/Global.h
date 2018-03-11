#ifndef GLOBAL_H
#define GLOBAL_H

/* ------------------------------------
 * Settings for the program.
 */

#ifndef MAX_COMMAND_BATCH
#  define MAX_COMMAND_BATCH 8
#endif
#ifndef MAX_GL_PARAMS
#  define MAX_GL_PARAMS 32
#endif

struct ParseParamsResult;
#define COMMAND(name,fmt) bool cmd_##name(struct ParseParamsResult *parsed)

#ifndef NDEBUG
#  define DEBUGMSG(a) DebugMsg a
  void DebugMsg(char *str, ...);
#else
  #define DEBUGMSG(a) do{}while(0)
#endif

#ifdef _WIN32
void WinPerror(char *msg);
#endif

#endif
