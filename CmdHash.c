#include "SymbolHash.h"
#include "Client.h"

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

