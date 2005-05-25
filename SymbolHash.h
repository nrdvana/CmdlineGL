#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

#include <stdio.h>

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
	union {
		int Value;
		void *Data;
	};
	int Type;
	RBTreeNode node;
} SymbVarEntry;

#define NAMED_LIST 0
#define NAMED_QUADRIC 1
#define NAMED_TEXTURE 2
#define NAMED_FONT 3

extern const char *SymbVarTypeName[];

const CmdHashEntry *GetCmd(const char *Key);
void DumpCommandList(FILE* DestStream);

const IntConstHashEntry *GetIntConst(const char *Key);
void DumpConstList(FILE* DestStream);

SymbVarEntry *CreateSymbVar(const char *Name);
const SymbVarEntry *GetSymbVar(const char *Name);
void DumpVarList(FILE* DestStream);

#define CmdLookupSize 64
struct CmdLookupBucket { int EntryCount; CmdHashEntry *Entries; };
extern const struct CmdLookupBucket CmdLookup[CmdLookupSize];

#define IntConstLookupSize 1024
struct IntConstLookupBucket { int EntryCount; IntConstHashEntry *Entries; };
extern const struct IntConstLookupBucket IntConstLookup[IntConstLookupSize];

#endif

