#ifndef PROCESS_INPUT_H
#define PROCESS_INPUT_H

#include "Global.h"

#define P_SUCCESS 0
#define P_IGNORE 1
#define P_CMD_ERR 2
#define P_NOT_FOUND 3
#define P_EOF 4

#ifndef _WIN32
void InitLineBuffer(int FileHandle);
#else
void InitLineBuffer(HANDLE FileHandle);
#endif
char* ReadLine();

int ProcessCommand(char *Line);

struct SymbVarEntry_t;


/* The ParseParams function family returns all parsed values into a collection
 * of fixed-size arrays.  So, a format of "iiff" will store two items into the
 * array of ints, and two items into the array of floats.  This saves a lot of
 * bookkeeping over dynamic allocation, or messing with unions, or trusting that
 * the union is of the type you think it is.
 */

#include "GlHeaders.h"

#define PARSEPARAMS_MAX_OBJECTS 32
#define PARSEPARAMS_MAX_FLOATS  32
#define PARSEPARAMS_MAX_STRINGS 32
#define PARSEPARAMS_MAX_INTS    32

struct ParseParamsResult {
	//long     longs[PARSEPARAMS_MAX_INTS];
	GLint    ints [PARSEPARAMS_MAX_INTS];
	GLfloat  floats[PARSEPARAMS_MAX_FLOATS];
	GLdouble doubles[PARSEPARAMS_MAX_FLOATS];
	char*    strings[PARSEPARAMS_MAX_STRINGS];
	struct SymbVarEntry_t *objects[PARSEPARAMS_MAX_OBJECTS];
	int iCnt, lCnt, fCnt, dCnt, sCnt, oCnt;
	char *errmsg, errmsg_buf[128];
};

extern bool ParseParams  (char **line, const char *format, struct ParseParamsResult *result);
extern char* next_token  (char **line);
extern bool ParseToken   (char **line, struct ParseParamsResult *result);
extern bool ParseInt     (char **line, struct ParseParamsResult *result);
//extern bool ParseLong    (char **line, struct ParseParamsResult *result);
extern bool ParseFloat   (char **line, struct ParseParamsResult *result);
extern bool ParseDouble  (char **line, struct ParseParamsResult *result);
extern bool ParseColor   (char **line, struct ParseParamsResult *result);
extern bool FileExists   (const char *Name);
extern bool CaptureRemainder(char **line, struct ParseParamsResult *result);

#endif
