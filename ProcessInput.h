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

typedef union ParamUnion_u {
	long as_long;
	float as_color[4];
	double as_double;
	struct SymbVarEntry_t *as_sym;
	char *as_str;
} ParamUnion;

typedef ParamUnion ParamList[MAX_GL_PARAMS];

extern bool ParseParamsCapErr(char *line, const char *format, ParamUnion *params, int *n_params, char* errbuf, int errbufsize);
extern bool ParseParams(char *command, char *line, const char *format, ParamUnion *params, int *n_params);
extern bool ParseLong(const char* Text, long *Result);
extern bool ParseDouble(const char* Text, double *Result);
extern bool ParseFloat(const char *Text, float *Result);
extern bool ParseColor(char **line, float color[4]);
extern bool ParseHexColor(const char* Text, float Result[4]);
extern bool ParseSymbVar(const char* Text, int Type, bool autocreate, struct SymbVarEntry_t **Result);
extern bool FileExists(const char *Name);

#endif
