#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

struct CmdHashEntry {
	char* Key;
	int Value;
};

struct IntConstHashEntry {
	char* Key;
	int Value;
};

struct CmdHashEntry *GetCmd(char *Key);

struct IntConstHashEntry *GetIntConst(char *Key);

#endif
