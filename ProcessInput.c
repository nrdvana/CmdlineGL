#include <sys/un.h>
#include <stdio.h>
#include "ProcessInput.h"
#include "Global.h"
#include "SymbolHash.h"

#define CMD_LEN_MAX 256
#define ARG_COUNT_MAX 32
#define READ_BUFFER_SIZE 1024

bool ReadCommand(int fd, int *ArgCountResult, char **ArgResult);
char* Readline(int fd);

int ServerConn;
struct sockaddr_un ServerAddr;
void *CmdData;
int CmdDataLen;
char LineBuffer[CMD_LEN_MAX];

int ProcessFD(int fd) {
	const CmdHashEntry *Cmd;
	int TokenCount;
	char *TokenPointers[ARG_COUNT_MAX];

	DEBUGMSG("Read command: ");
	if (ReadCommand(fd, &TokenCount, TokenPointers)) {
		DEBUGMSG("Check token count.. ");
		if (TokenCount > 0) {
			DEBUGMSG("Look up function.. ");
			Cmd= GetCmd(TokenPointers[0]);
			if (Cmd != NULL) {
				DEBUGMSG("\"%s\".\n", Cmd->Key);
				Cmd->Value(TokenCount-1, TokenPointers+1); // run command
				return P_SUCCESS;
			}
			else {
				DEBUGMSG("Not found.\n");
				return P_NOT_FOUND;
			}
		}
		else
			return P_CMD_ERR;
	}
	else
		return P_EOF;
}

bool ReadCommand(int fd, int *ArgCountResult, char **ArgResult) {
	char *LastArgPos, *temp, *Line;

	Line= Readline(fd);
	if (!Line) return false;

	DEBUGMSG("Parse command.. ");
	*ArgCountResult= 0;
	LastArgPos= NULL;
	temp= strtok(Line, " \t");
	while (temp != NULL && *ArgCountResult < ARG_COUNT_MAX) {
		if (temp > LastArgPos + 1)
			ArgResult[(*ArgCountResult)++]= temp;
		LastArgPos= temp;
		temp= strtok(NULL, " \t");
	}
	return true;
}

char ReadBuffer[READ_BUFFER_SIZE];
char const *StopPos= ReadBuffer+READ_BUFFER_SIZE;
char *Pos= ReadBuffer;
char *DataPos= ReadBuffer;

char* Readline(int fd) {
	int shift, red;
	char* LineStart= Pos;
	do {
//		DEBUGMSG("LineStart = %d, Pos = %d, DataPos = %d, StopPos = %d\n", LineStart-ReadBuffer, Pos-ReadBuffer, DataPos-ReadBuffer, StopPos-ReadBuffer);
		if (Pos == DataPos) {
			red= read(fd, DataPos, StopPos-DataPos);
			if (red <= 0)
				break;
			DataPos+= red;
		}
		while (Pos < DataPos && *Pos != '\n')
			Pos++;
		// Pos is now either at DataPos or a newline character, or both
		if (Pos == StopPos && LineStart != ReadBuffer) {
			shift= LineStart - ReadBuffer;
			memmove(ReadBuffer, LineStart, Pos-LineStart);
			Pos-= shift;
			DataPos-= shift;
			LineStart-= shift;
		}
	} while (*Pos != '\n' && Pos < StopPos);
	
	if (Pos == LineStart)
		return NULL;
	if (Pos == StopPos) {
		fprintf(stderr, "Input line too long\n");
		return NULL;
	}
	*Pos++= '\0';
	return LineStart;
}


