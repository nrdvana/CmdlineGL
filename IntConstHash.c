#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "SymbolHash.h"

#include "IntConstHash.autogen.c"

struct IntConstHashEntry *GetIntConst(char *Key) {
	int code;
	if (Key[0] == 'G' && Key[1] == 'L') {
		Key+= 2;
		code= CalcHash(Key, IntConstLookupSize);
		for (i=CmdLookup[code].EntryCount-1; i>=0; i--) {
			if (strcmp(CmdLookup[code].Entries[i].Key, Key) == 0)
				return &CmdLookup[code].Entries[i].Value;
		}
	}
	return (struct IntConstHashEntry *) 0;
}
