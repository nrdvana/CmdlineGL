#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Global.h"
#include "SymbolHash.h"
#include "ParseGL.h"
#include "Server.h"

int CalcHash(const char* String) {
	int Result= 0;
	const char *Pos= String;
	while (*Pos)
		Result= ((Result<<1) + (Result ^ *Pos++));
	return Result;
}

#include "CmdHash.autogen.c"

const CmdHashEntry *GetCmd(const char *Key) {
	int code, i;

	code= CalcHash(Key) & (CmdLookupSize-1); // mask it to table size
	for (i=CmdLookup[code].EntryCount-1; i>=0; i--) {
		if (strcmp(CmdLookup[code].Entries[i].Key, Key) == 0)
			return &CmdLookup[code].Entries[i];
	}
	return (CmdHashEntry*) 0;
}

#include "IntConstHash.autogen.c"

const IntConstHashEntry *GetIntConst(const char *Key) {
	int code, i;
	if (Key[0] == 'G' && Key[1] == 'L') {
		Key+= 2;
		code= CalcHash(Key) & (IntConstLookupSize-1); // mask it to the table size
		for (i=IntConstLookup[code].EntryCount-1; i>=0; i--) {
			if (strcmp(IntConstLookup[code].Entries[i].Key, Key) == 0)
				return &IntConstLookup[code].Entries[i];
		}
	}
	return (IntConstHashEntry *) 0;
}

bool SymbVarTreeInit= false;
RBTree SymbVarTree;

typedef struct {
	int Hash;
	const char *Name;
} SymbVarSearchKey;

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
	
	if (!SymbVarTreeInit) {
		RBTree_InitRootSentinel(&SymbVarTree.RootSentinel);
		SymbVarTreeInit= true;
	}
	Key.Hash= CalcHash(Name);
	Key.Name= Name;
	RBTreeNode *Node= RBTree_Find(&SymbVarTree.RootSentinel, &Key, SymbVar_compare_func);
	if (Node) return Node->Object;
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
