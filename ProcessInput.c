#include <sys/socket.h>
#include <sys/un.h>
#include "Global.h"
#include "SymbolHash.h"

#define CMD_LEN_MAX 256
#define ARG_COUNT_MAX 32
#define READ_BUFFER_SIZE 1024

bool ReadCommand(int *ArgCountResult, char **ArgResult);
void ProcessCommand(int ArgCount, char** Args);
void SendCommand();
char* Readline();

int ServerConn;
struct sockaddr_un ServerAddr;
char StandardCmdBuffer[64];
void *CmdData;
int CmdDataLen;
char LineBuffer[CMD_LEN_MAX];

void ExecClient(char *SocketName, int argc, char **argv) {
	int CmdArgCount;
	char *CmdArgs[ARG_COUNT_MAX];

	// build the address
	Addr.sun_family= AF_UNIX;
	if (strlen(SocketName) >= sizeof(Addr.sun_path))
		Err_InvalidSocket();
	strcpy(SocketName, Addr.sun_path);

	// try connecting
	ServerConn= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ServerConn < 0)
		Err_InvalidSocket();

	if (argc > 0)
		ProcessCommand(argc, argv);

	while (ReadCommand(&CmdArgCount, CmdArgs))
		ProcessCommand(CmdArgCount, CmdArgs);

	close(ServerConn);
}

void ProcessCommand(int ArgCount, char** Args) {
	struct CmdHashEntry *Cmd;

	if (ArgCount > 0) {
		Cmd= GetCmd(Args[0]);
		if (Cmd != NULL) {
			CmdData= StandardCmdbuffer;
			Cmd->Value(ArgCount-1, Args+1); // encode command
			SendCommand(Data);
			if (CmdData != StandardCmdBuffer)
				free(CmdData);
		}
		else
			Err_BadCommand();
	}
	else
		Err_BadCommand();
}

void SendCommand(void *Data) {
	sendto(ServerConn, CmdData, CmdDataLen, (struct sockaddr*) ServerAddr, sizeof(ServerAddr));
}

bool ReadCommand(int *ArgCountResult, char **ArgResult) {
	char *LastArg, *temp, *Line;

	Line= Readline();
	if (!Line) return false;

	*ArgCountResult= 0;
	LastArg= NULL;
	temp= strtok(Line, " \t");
	while (temp != NULL && *ArgCountResult < ARG_COUNT_MAX) {
		if (temp > LastArg + 1)
			ArgResult[(*ArgCountResult)++]= temp;
		LastArg= temp;
		temp= strtok(NULL, " \t");
	}
	return true;
}

char ReadBuffer[READ_BUFFER_SIZE];
char const *StopPos= ReadBuffer+READ_BUFFER_SIZE-1;
char *Pos= ReadBuffer;
char *DataPos= ReadBuffer;

char* Readline() {
	int shift, red;
	char* LineStart= Pos;
	do {
		if (Pos == DataPos) {
			red= read(0, DataPos, StopPos-DataPos);
			if (red <= 0)
				break;
			DataPos+= red;
		}
		*DataPos= '\0';
		Pos= strchr(Pos, '\n');
		if (Pos == StopPos && LineStart != ReadBuffer) {
			memmove(ReadBuffer, LineStart, Pos-LineStart);
			shift= LineStart - ReadBuffer;
			Pos-= shift;
			DataPos-= shift;
			LineStart-= shift;
		}
	} while (*Pos != '\n' && Pos < StopPos);
	
	if (Pos == LineStart)
		return NULL;
	if (Pos == StopPos) {
		Err_LineTooLong();
		return NULL;
	}
	*Pos= '\0';
	return LineStart;
}


