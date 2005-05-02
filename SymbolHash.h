#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

typedef struct {
	const char* Key;
	int (*Value)(int,char**);
} CmdHashEntry;

typedef struct {
	const char* Key;
	int Value;
} IntConstHashEntry;

#define SYMB_VAR_MAX_LEN 32
#include "Contained_RBTree.h"
typedef struct SymbVarEntry_t {
	int Hash;
	char Name[SYMB_VAR_MAX_LEN];
	int Value;
	int Type;
	RBTreeNode node;
} SymbVarEntry;

#define NAMED_LIST 0
#define NAMED_QUADRIC 1

const CmdHashEntry *GetCmd(const char *Key);

const IntConstHashEntry *GetIntConst(const char *Key);

SymbVarEntry *CreateSymbVar(const char *Name);
const SymbVarEntry *GetSymbVar(const char *Name);

#endif
