#include <sys/socket.h>
#include <sys/un.h>
#include "Global.h"
#include "Client.h"
#include "SymbolHash.h"

socket ServerConn;

#define CMD_LEN_MAX 256
#define ARG_COUNT_MAX 8
#define READ_BUFFER_SIZE 1024

void ExecClient(char *SocketName, int argc, char **argv) {
	struct sockaddr_un Addr;
	char Buffer[CMD_LEN_MAX];
	char *NewArgs[ARG_COUNT_MAX];

	// build the address
	Addr.sa_family_t= AF_UNIX;
	if (strlen(SocketName) >= sizeof(Addr.sun_path))
		Err_InvalidSocket();
	strcpy(SocketName, Addr.sun_path);

	// try connecting
	ServerConn= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ServerConn < 0)
		Err_InvalidSocket();

	if (argc > 0)
		SendCommand(argc, argv);

	while (ReadCommand(Buffer, &argc, NewArgs))
		SendCommand(argc, NewArgs);

	close(ServerConn);
}

void SendCommand(int ArgCount, char **Args) {
	if (ArgCount == 0)
		Err_BadCommand();
	else {
		CmdHashEntry *Cmd= GetCmd(Args[0]);
		if (Cmd != null)
			Cmd->Value(ArgCount-1, Args+1);
		else
			Err_BadCommand();
	}
}

bool ReadCommand(int *ArgCountResult, char **ArgResult) {
	char *LastArg, *temp, *Line;

	Line= ReadLine();
	if (!Line) return false;

	*ArgCountResult= 0;
	LastArg= null;
	temp= strtok(ArgResult, " \t");
	while (temp != null && *ArgCountResult < ARG_COUNT_MAX) {
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
char *DataPos= Pos;

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
		if (Pos == StopPos && LineStart != LineBuffer) {
			memmove(ReadBuffer, LineStart, Pos-LineStart);
			shift= LineStart - ReadBuffer;
			Pos-= shift;
			DataPos-= shift;
			LineStart-= shift;
		}
	} while (*Pos != '\n' && Pos < StopPos);
	
	if (Pos == LineStart)
		return null;
	if (Pos == StopPos) {
		Err_LineTooLong();
		return null;
	}
	*Pos= '\0';
	return LineStart;
}



