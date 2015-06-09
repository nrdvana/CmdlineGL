#include "Global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SymbolHash.h"

const char *SymbVarTypeName[]= { "Display List", "Quadric", "Texture", "Font" };
bool SymbVarTreeInit= false;
RBTree SymbVarTree;

const CmdHashEntry *GetCmd(const char *Key) {
	int code, i;

	code= CalcHash(Key) & (CmdLookupSize-1); // mask it to table size
	for (i=CmdLookup[code].EntryCount-1; i>=0; i--) {
		if (strcmp(CmdLookup[code].Entries[i].Key, Key) == 0)
			return &CmdLookup[code].Entries[i];
	}
	return NULL;
}

const IntConstHashEntry *GetIntConst(const char *Key) {
	int code, i;
	code= CalcHash(Key) & (IntConstLookupSize-1); // mask it to the table size
	for (i=IntConstLookup[code].EntryCount-1; i>=0; i--) {
		if (strcmp(IntConstLookup[code].Entries[i].Key, Key) == 0)
			return &IntConstLookup[code].Entries[i];
	}
	return NULL;
}

void InitSymbVarEntry(SymbVarEntry *Entry, const char* Name) {
	Entry->Hash= CalcHash(Name);
	strncpy(Entry->Name, Name, SYMB_VAR_MAX_LEN-1);
	Entry->Name[SYMB_VAR_MAX_LEN-1]= '\0';
	RBTreeNode_Init(&(Entry->node));
	Entry->node.Object= Entry;
}

bool SymbVar_inorder_func(const void* ObjA, const void* ObjB) {
	SymbVarEntry *EntryA= (SymbVarEntry*) ObjA;
	SymbVarEntry *EntryB= (SymbVarEntry*) ObjB;
	if (EntryA->Hash == EntryB->Hash)
		return strcmp(EntryA->Name, EntryB->Name) <= 0;
	else
		return (EntryA->Hash <= EntryB->Hash);
}

/** Structure used to pass the key to the compare function */
typedef struct {
	int Hash;
	const char *Name;
} SymbVarSearchKey;

int SymbVar_compare_func(const void* SearchKey, const void* Object) {
	SymbVarSearchKey *Key= (SymbVarSearchKey*) SearchKey;
	SymbVarEntry *Node= (SymbVarEntry*) Object;
	if (Key->Hash == Node->Hash)
		return strcmp(Key->Name, Node->Name);
	else
		return Key->Hash - Node->Hash;
}

const SymbVarEntry *GetSymbVar(const char *Name) {
	SymbVarSearchKey Key;
	RBTreeNode *Node;

	if (!SymbVarTreeInit) {
		RBTree_InitRootSentinel(&SymbVarTree.RootSentinel);
		SymbVarTreeInit= true;
	}
	Key.Hash= CalcHash(Name);
	Key.Name= Name;
	Node= RBTree_Find(&SymbVarTree.RootSentinel, &Key, SymbVar_compare_func);
	if (Node) return (SymbVarEntry*) Node->Object;
	else return NULL;
}

SymbVarEntry *CreateSymbVar(const char *Name) {
	SymbVarEntry *Entry;
	if (!SymbVarTreeInit) {
		RBTree_InitRootSentinel(&SymbVarTree.RootSentinel);
		SymbVarTreeInit= true;
	}
	Entry= (SymbVarEntry*) malloc(sizeof(SymbVarEntry));
	InitSymbVarEntry(Entry, Name);
	RBTree_Add(&SymbVarTree.RootSentinel, &(Entry->node), SymbVar_inorder_func);
	return Entry;
}

void DeleteSymbVar(SymbVarEntry *Entry) {
	RBTree_Prune(&Entry->node);
	free(Entry);
}

void DumpCommandList(FILE* DestStream) {
	int i, j;
	for (i=0; i<CmdLookupSize; i++)
		for (j=0; j<CmdLookup[i].EntryCount; j++)
			fprintf(DestStream, "%s\n", CmdLookup[i].Entries[j].Key);
}
void DumpConstList(FILE* DestStream) {
	int i, j;
	for (i=0; i<IntConstLookupSize; i++)
		for (j=0; j<IntConstLookup[i].EntryCount; j++)
			fprintf(DestStream, "%s %d\n", IntConstLookup[i].Entries[j].Key, IntConstLookup[i].Entries[j].Value);
}
void DumpVarList(FILE* DestStream) {
	RBTreeNode *Current= RBTree_GetLeftmost(SymbVarTree.RootSentinel.Left);
	while (Current != &Sentinel) {
		fprintf(DestStream, "%s %s\n", ((SymbVarEntry*)Current->Object)->Name, SymbVarTypeName[((SymbVarEntry*)Current->Object)->Type]);
		Current= RBTree_GetNext(Current);
	}
}

