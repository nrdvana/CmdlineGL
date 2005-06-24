#include <sys/un.h>
#include <stdio.h>
#include "Global.h"
#include "GlHeaders.h"
#include "ProcessInput.h"
#include "SymbolHash.h"

#define CMD_LEN_MAX 256
#define READ_BUFFER_SIZE 1024
#define TOK_COUNT_MAX MAX_GL_PARAMS+1

int ServerConn;
struct sockaddr_un ServerAddr;
void *CmdData;
int CmdDataLen;
char LineBuffer[CMD_LEN_MAX];

bool ParseLine(char *Line, int *ArgCountResult, char **ArgResult) {
	char *LastArgPos, *temp;

	*ArgCountResult= 0;
	LastArgPos= NULL;
	temp= strtok(Line, " \t");
	while (temp != NULL && *ArgCountResult < TOK_COUNT_MAX) {
		if (temp > LastArgPos + 1)
			ArgResult[(*ArgCountResult)++]= temp;
		LastArgPos= temp;
		temp= strtok(NULL, " \t");
	}
	ArgResult[*ArgCountResult]= NULL;
	return *ArgCountResult > 0;
}

int ProcessCommand(char **TokenPointers, int TokenCount) {
	const CmdHashEntry *Cmd;
	int Result;
	
	Cmd= GetCmd(TokenPointers[0]);
	if (Cmd != NULL) {
		Result= Cmd->Value(TokenCount-1, TokenPointers+1); // run command
		switch (Result) {
		case 0:
			if (IsGlBegun) // can't check command success
				return 0;
			Result= glGetError();
			if (Result == GL_NO_ERROR || Result == 0) // command was successful
				return 0;
			else do {
				fprintf(stderr, "GL error while executing %s: %s.\n", TokenPointers[0], gluErrorString(Result));
				Result= glGetError();
			} while (Result != GL_NO_ERROR && Result != 0);
			break;
		case ERR_PARAMCOUNT:
			fprintf(stderr, "Wrong number of parameters for %s\n", TokenPointers[0]);
			break;
		case ERR_PARAMPARSE:
			fprintf(stderr, "Cannot parse parameters for %s\n", TokenPointers[0]);
			break;
		default:
			fprintf(stderr, "Unknown result code: %d\n", Result);
		}
		return P_CMD_ERR;
	}
	else {
		fprintf(stderr, "Unknown command: \"%s\".\n", TokenPointers[0]);
		return P_NOT_FOUND;
	}
}

char ReadBuffer[READ_BUFFER_SIZE];
char const *StopPos= ReadBuffer+READ_BUFFER_SIZE;
char *Pos= ReadBuffer;
char *DataPos= ReadBuffer;
char *LineStart= ReadBuffer;

void ShiftBuffer() {
	int shift= LineStart - ReadBuffer;
	if (DataPos != LineStart)
		memmove(ReadBuffer, LineStart, DataPos - LineStart);
	DataPos-= shift;
	Pos-= shift;
	LineStart= ReadBuffer;
}

char* ReadLine(int fd) {
	int red;
	char *Result;

	if (LineStart != ReadBuffer && DataPos - LineStart < 16)
		ShiftBuffer();
	do {
		// do we need more?
		if (Pos == DataPos) {
			// do we need to shift?
			if (DataPos == StopPos) {
				// can we shift?
				if (LineStart != ReadBuffer)
					ShiftBuffer();
				else {
					// Full buffer.  Abort, and reset the buffer pointers.
					LineStart= DataPos= Pos= ReadBuffer;
					fprintf(stderr, "Input line too long\n");
					return NULL;
				}
			}
			red= read(fd, DataPos, StopPos-DataPos);
			if (red <= 0) {
				if (red == 0)
					DEBUGMSG(("Read 0 bytes.\n"));
				else {
#ifdef DEBUG
					if (errno == EAGAIN)
						fprintf(stderr, ".");
					else
						perror("read");
#endif
				}
				return NULL;
			}
			DataPos+= red;
		}
		while (Pos < DataPos && *Pos != '\n')
			Pos++;
//		DEBUGMSG(("LineStart = %d, Pos = %d, DataPos = %d, StopPos = %d\n", LineStart-ReadBuffer, Pos-ReadBuffer, DataPos-ReadBuffer, StopPos-ReadBuffer));
	} while (*Pos != '\n');
	*Pos++= '\0';
	Result= LineStart;
	LineStart= Pos;
	return Result;
}


