#ifndef SYMBOL_HASH_H
#define SYMBOL_HASH_H

#include "ProcessInput.h"

struct ParseParamsResult;

typedef struct {
	const char* Name;
	const char* ArgFormat;
	bool (*Handler)(struct ParseParamsResult *parsed);
} CmdListEntry;

typedef struct {
	const char* Name;
	int Value;
} IntConstListEntry;

#ifndef SYMB_VAR_MAX_LEN
#define SYMB_VAR_MAX_LEN 32
#endif
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

#define NAMED_LIST 1
#define NAMED_QUADRIC 2
#define NAMED_TEXTURE 3
#define NAMED_FONT 4

extern const char *SymbVarTypeName[];

extern const CmdListEntry *GetCmd(const char *Name);
extern void DumpCommandList(FILE* DestStream);

extern const IntConstListEntry *GetIntConst(const char *Name);
extern void DumpConstList(FILE* DestStream);

extern SymbVarEntry *CreateSymbVar(const char *Name, int Type);
extern SymbVarEntry *GetSymbVar(const char *Name, int Type);
extern void DeleteSymbVar(SymbVarEntry *Entry); // does not free contents of the variable!!
extern void DumpVarList(FILE* DestStream);

#endif

