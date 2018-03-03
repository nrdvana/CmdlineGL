#include "Global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SymbolHash.h"

const char *SymbVarTypeName[]= { "Display List", "Quadric", "Texture", "Font" };
bool SymbVarTreeInit= false;
RBTree SymbVarTree;

extern int CmdHashFunc(const char *Str);
extern const int CmdHashTableSize;
extern const CmdHashEntry CmdHashTable[];
extern int IntConstHashFunc(const char *Str);
extern const int IntConstHashTableSize;
extern const IntConstHashEntry IntConstHashTable[];

const CmdHashEntry *GetCmd(const char *Name) {
	int code= CmdHashFunc(Name);
	if (CmdHashTable[code].Name && strcmp(CmdHashTable[code].Name, Name) == 0)
		return &CmdHashTable[code];
	else
		return NULL;
}

const IntConstHashEntry *GetIntConst(const char *Name) {
	int code= IntConstHashFunc(Name);
	int lim= code + 5;
	while (code < lim) {
		if (IntConstHashTable[code].Name && strcmp(IntConstHashTable[code].Name, Name) == 0)
			return &IntConstHashTable[code];
		code++;
	}
	return NULL;
}

unsigned int CalcHash(const char* str) {
	unsigned int Result= 0;
	while (*str)
		Result= ((Result<<1) + (Result ^ *str++));
	return Result;
}

void InitSymbVarEntry(SymbVarEntry *Entry, const char* Name, int Type) {
	Entry->Hash= Type+CalcHash(Name);
	Entry->Type= Type;
	strncpy(Entry->Name, Name, SYMB_VAR_MAX_LEN-1);
	Entry->Name[SYMB_VAR_MAX_LEN-1]= '\0';
	RBTreeNode_Init(&(Entry->node));
	Entry->node.Object= Entry;
}

bool SymbVar_inorder_func(const void* ObjA, const void* ObjB) {
	SymbVarEntry *EntryA= (SymbVarEntry*) ObjA;
	SymbVarEntry *EntryB= (SymbVarEntry*) ObjB;
	if (EntryA->Hash == EntryB->Hash) {
		if (EntryA->Type == EntryB->Type)
			return strcmp(EntryA->Name, EntryB->Name) <= 0;
		else
			return EntryA->Type < EntryB->Type;
	}
	else
		return (EntryA->Hash < EntryB->Hash);
}

/** Structure used to pass the key to the compare function */
typedef struct {
	int Hash;
	const char *Name;
	int Type;
} SymbVarSearchKey;

int SymbVar_compare_func(const void* SearchKey, const void* Object) {
	SymbVarSearchKey *Key= (SymbVarSearchKey*) SearchKey;
	SymbVarEntry *Node= (SymbVarEntry*) Object;
	if (Key->Hash == Node->Hash) {
		if (Key->Type == Node->Type)
			return strcmp(Key->Name, Node->Name);
		else
			return Key->Type - Node->Type;
	}
	else
		return Key->Hash - Node->Hash;
}

SymbVarEntry *GetSymbVar(const char *Name, int Type) {
	SymbVarSearchKey Key;
	RBTreeNode *Node;

	if (!SymbVarTreeInit) {
		RBTree_InitRootSentinel(&SymbVarTree.RootSentinel);
		SymbVarTreeInit= true;
	}
	Key.Hash= Type+CalcHash(Name);
	Key.Name= Name;
	Key.Type= Type;
	Node= RBTree_Find(&SymbVarTree.RootSentinel, &Key, SymbVar_compare_func);
	if (Node) return (SymbVarEntry*) Node->Object;
	else return NULL;
}

SymbVarEntry *CreateSymbVar(const char *Name, int Type) {
	SymbVarEntry *Entry;
	if (!SymbVarTreeInit) {
		RBTree_InitRootSentinel(&SymbVarTree.RootSentinel);
		SymbVarTreeInit= true;
	}
	Entry= (SymbVarEntry*) malloc(sizeof(SymbVarEntry));
	InitSymbVarEntry(Entry, Name, Type);
	RBTree_Add(&SymbVarTree.RootSentinel, &(Entry->node), SymbVar_inorder_func);
	return Entry;
}

void DeleteSymbVar(SymbVarEntry *Entry) {
	RBTree_Prune(&Entry->node);
	free(Entry);
}

void DumpCommandList(FILE* DestStream) {
	int i;
	for (i=0; i<CmdHashTableSize; i++)
		if (CmdHashTable[i].Name)
			fprintf(DestStream, "%s\n", CmdHashTable[i].Name);
}
void DumpConstList(FILE* DestStream) {
	int i;
	for (i=0; i<IntConstHashTableSize; i++)
		if (IntConstHashTable[i].Name)
			fprintf(DestStream, "%s %d\n", IntConstHashTable[i].Name, IntConstHashTable[i].Value);
}
void DumpVarList(FILE* DestStream) {
	RBTreeNode *Current= RBTree_GetLeftmost(SymbVarTree.RootSentinel.Left);
	while (Current != &Sentinel) {
		fprintf(DestStream, "%s %s\n",
			SymbVarTypeName[((SymbVarEntry*)Current->Object)->Type],
			((SymbVarEntry*)Current->Object)->Name
		);
		Current= RBTree_GetNext(Current);
	}
}

