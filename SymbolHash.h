#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

typedef struct {
	const char* Key;
	void (*Value)(int,char**);
} CmdHashEntry;

typedef struct {
	const char* Key;
	int Value;
} IntConstHashEntry;

const CmdHashEntry *GetCmd(const char *Key);

const IntConstHashEntry *GetIntConst(const char *Key);

#endif
