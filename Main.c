#include "Global.h"
#include "Server.h"
#include "Client.h"

void PrintUsage();

int main(int argc, char **argv) {
	int arg= 1, WantServer= 0, WantHelp= 0, NeedHelp= 0;

	// Read through and flags or switches
	if (argc == 1) WantHelp= 1;
	while (arg < argc && argv[arg][0] == '-') {
		if (strcmp(argv[arg], "-c") == 0)
			WantServer= 1;
		else if (strcmp(argv[arg], "-h") == 0)
			WantHelp= 1;
		else
			NeedHelp= 1;
		arg++;
	}

	// Pick an action
	if (WantServer) {
		if (argc - arg == 1)
			StartServer(argv[arg]);
		else {
			PrintUsage();
			return -1;
		}
	}
	else if (WantHelp || NeedHelp) {
		ExitWithUsage();
		return WantHelp? 0 : -1;
	}
	else {
		ExecClient(argv[arg], argc-1-arg, argv+1+arg);
	}
	return 0;
}

void PrintUsage() {
	printf("Usage:  CmdlineGL -c socketname\n");
	printf(      "\tCmdlineGL socketname command [param ...]\n");
	printf(      "\tcommand-stream | CmdlineGL sockname\n");
	printf("\n");
	printf("Where socketname is the path/filename for a named socket\n");
	printf(" and command is a text command with optional text parameters\n");
	printf("In the streamed mode, each line of input is broken on space\n");
	printf(" characters and treated as a command.  There is currently no\n");
	printf(" escaping mechanism, although I'm not opposed to the idea.\n");
}
