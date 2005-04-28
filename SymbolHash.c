#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Global.h"
#include "SymbolHash.h"
#include "ParseGL.h"

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
