#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

struct CmdHashEntry {
	char* String;
	int Code;
};

struct IntConstHashEntry {
	char* String;
	int Code;
};

struct CmdHashEntry *GetCmd(char *Key);

struct IntConstHashEntry *GetIntConst(char *Key);

#endif
