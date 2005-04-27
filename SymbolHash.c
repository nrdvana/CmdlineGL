#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "Client.h"
#include "SymbolHash.h"

int CalcHash(char* String, int Mod) {
	int Result= 0;
	int Mask= Mod-1;
	char *Pos= String;
	while (*Pos)
		Result= ((Result<<4) + (Result ^ *Pos++))&Mask;
	return Result;
}

#include "CmdHash.autogen.c"

struct CmdHashEntry *GetCmd(char *Key) {
	int code, i;

	code= CalcHash(Key, CmdLookupSize);
	for (i=CmdLookup[code].EntryCount-1; i>=0; i--) {
		if (strcmp(CmdLookup[code].Entries[i].Key, Key) == 0)
			return &CmdLookup[code].Entries[i];
	}
	return (struct CmdHashEntry*) 0;
}

#include "IntConstHash.autogen.c"

struct IntConstHashEntry *GetIntConst(char *Key) {
	int code, i;
	if (Key[0] == 'G' && Key[1] == 'L') {
		Key+= 2;
		code= CalcHash(Key, IntConstLookupSize);
		for (i=IntConstLookup[code].EntryCount-1; i>=0; i--) {
			if (strcmp(IntConstLookup[code].Entries[i].Key, Key) == 0)
				return &IntConstLookup[code].Entries[i];
		}
	}
	return (struct IntConstHashEntry *) 0;
}
