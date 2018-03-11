#include <config.h>
#include "Global.h"
#include "SymbolHash.h"

const char *SymbVarTypeName[]= { "?", "Display List", "Quadric", "Texture", "Font" };
bool SymbVarTreeInit= false;
RBTree SymbVarTree;

/* ConstList dynamically generated variables */
extern const int IntConstListCount;
extern const IntConstListEntry IntConstList[];
extern const int IntConstHashTableSize;
extern const uint16_t IntConstHashTable[];

void DumpConstList(FILE* DestStream) {
	int i;
	for (i=1; i<=IntConstListCount; i++) /* 1-based list, since hashtable 0 isn't a valid item */
		if (IntConstList[i].Name)
			fprintf(DestStream, "%s %d\n", IntConstList[i].Name, IntConstList[i].Value);
}

/* CmdList dynamically generated variables */
extern const int CmdListCount;
extern const CmdListEntry CmdList[];
extern const int CmdHashTableSize;
extern const uint16_t CmdHashTable[];

void DumpCommandList(FILE* DestStream) {
	int i;
	for (i=1; i<=CmdListCount; i++) /* 1-based list, since hashtable 0 isn't a valid item */
		if (CmdList[i].Name)
			fprintf(DestStream, "%s\n", CmdList[i].Name);
}

/* The rest is related to the red/black tree that stored dynamic named objects */

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

