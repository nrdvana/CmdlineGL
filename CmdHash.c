#include "SymbolHash.h"

int CalcHash(char* String) {
	int Result= 0;
	char *Pos= String;
	while (*Pos)
		Result= ((Result<<3) ^ (Result>>5) ^ (*Pos++))&0xFF;
	return Result;
}

#include "CmdHash.autogen.c"

struct CmdHashEntry *GetCmd(char *Key) {
	int code, i;

	code= CalcHash(Key);
	for (i=CmdLookup[code].EntryCount-1; i>=0; i--) {
		if (strcmp(CmdLookup[code].Entries[i].Key, Key) == 0)
			return &CmdLookup[code].Entries[i];
	}
	return (struct CmdHashEntry*) 0;
}

/*
DATA Begin			1
DATA End			2
DATA LoadIdentity	3
DATA PushMatrix 	4
DATA PopMatrix		5
DATA Scale			6
DATA Translate		7
DATA Viewport		8
DATA MatrixMode 	9
DATA Ortho			10
DATA Frustum		11
DATA Enable 		12
DATA Disable		13
DATA Light			14
DATA uCylinder		30
DATA uSphere		31
DATA utSwapBuffers	32
DATA utInit 		33
DATA utInitDisplayMode 34
DATA utCreateWindow    35
DATA utInitWindowSize  36
*/


