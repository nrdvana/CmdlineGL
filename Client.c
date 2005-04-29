#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>

#include "Global.h"

char Buffer[1024];

bool ReadParams(char **Args, char **SocketNameResult, char ***RemainingArgs);
bool SendArgsAsCommand(char *CmdName, char **NextArg);
bool IsCalledAsCommand(const char *Name);

int ServerSock= 0;
struct sockaddr_un ServerAddr;
bool CalledAsCommand;

int main(int Argc, char**Args) {
	char *SocketName= NULL;
	char **NextArg;
	bool TerminateOnEOF= false;
	int len, pos, red, result;

	CalledAsCommand= IsCalledAsCommand(Args[0]);
	
	if (!ReadParams(Args, &SocketName, &NextArg))
		return 1;
	
	if (!SocketName)
		SocketName= getenv("CMDLINEGL_SOCKET");
		
	if (!SocketName) {
		fprintf(stderr, "No socket specified, and env var CMDLINEGL_SOCKET is not set.\n");
		return 2;
	}

	// build the address
	ServerAddr.sun_family= AF_UNIX;
	if (strlen(SocketName) >= sizeof(ServerAddr.sun_path)) {
		fprintf(stderr, "Socket name too long.\n");
		return false;
	}
	strcpy(ServerAddr.sun_path, SocketName);

	ServerSock= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ServerSock < 0) {
		perror("create socket");
		return false;
	}

	// if we have a GL function call on the arguments list, sent it
	if (*NextArg != NULL || CalledAsCommand) {
		SendArgsAsCommand(Args[0], NextArg);
	}
	// else pump commands from stdin.
	else do {
		red= read(0, Buffer, sizeof(Buffer), 0);
		if (red > 0)
			result= sendto(ServerSock, Buffer, red, 0, (struct sockaddr*) &ServerAddr, sizeof(ServerAddr));
		if (result < 0) {
			fprintf(stderr, "Cannot send to server socket file \"%s\".\n", SocketName);
			break;
		}
	} while (red > 0);

	close(ServerSock);
	
	return 0;
}

bool IsCalledAsCommand(const char *Name) {
	return Name[0] == 'g' && Name[1] == 'l' && Name[2] != '\0';
}

bool SendArgsAsCommand(char *CmdName, char **NextArg) {
	int pos= 0;
	int len;
	// if we manage to fill the 1K buffer, something is screwed up, so just cal it an error and exit.
	const char* OverflowStr= "Arguments too long.  Keep it under 1K please.";

	// If the called name of this program is a gl command (and not just "gl"), use that.
	if (CalledAsCommand) {
		len= strlen(CmdName);
		if (len+1 < sizeof(Buffer)) {
			memcpy(Buffer, CmdName, len);
			pos+= len;
			Buffer[pos++]= ' ';
		}
		else {
			// overflow check
			fprintf(stderr, OverflowStr);
			return false;
		}
	}
	// otherwise, do a simple check for bad parameters
	else if (NextArg[0][0] != 'g' || NextArg[0][1] != 'l') {
		fprintf(stderr, "All non-switch parameters must be part of an OpenGL command\n");
		return false;
	}
	// now attach the rest of the params
	while (*NextArg) {
		len= strlen(*NextArg);
		if (pos+len+1 > sizeof(Buffer)) {
			// overflow check
			fprintf(stderr, OverflowStr);
			return false;
		}
		memcpy(Buffer+pos, *NextArg, len);
		pos+= len;
		Buffer[pos++]= ' ';
		NextArg++;
	}
	// end the line (overwriting the space)
	Buffer[pos-1]= '\n';
	sendto(ServerSock, Buffer, pos, 0, (struct sockaddr*) &ServerAddr, sizeof(ServerAddr));
}

bool ReadParams(char **Args, char **SocketNameResult, char ***RemainingArgs) {
	char **NextArg= Args+1;
	bool ReadingArgs= true;
	// Note: this function takes advantage of the part of the program argument specification
	//   that says all argv lists must end with a NULL pointer.
	while (ReadingArgs && *NextArg && (*NextArg)[0] == '-') {
		switch ((*NextArg)[1]) {
		case 'l': *SocketNameResult= *++NextArg; break; // '-l BLAH' = listen on socket BLAH
		case '\0': ReadingArgs= false; break; // "-" = end of arguments
		case '-': // "--" => long-option
		default:
			fprintf(stderr, "Unrecognized argument: %s\n", *NextArg);
			return false;
		}
		NextArg++;
	}
	*RemainingArgs= NextArg;
	return true;
}


