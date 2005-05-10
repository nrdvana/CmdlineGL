#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <sys/time.h>
#include <unistd.h>

#include "Global.h"
#include "Server.h"
#include "ProcessInput.h"

bool Shutdown= false;

typedef struct {
	char *FifoName;
	bool TerminateOnEOF;
	bool WantHelp;
	bool NeedHelp;
} CmdlineOptions;

void ReadParams(char **args, CmdlineOptions *Options);
void PrintUsage();
bool CreateListenSocket(char *SocketName, int *ListenSocket);
void WatchSocket(int id);

long microseconds(struct timeval *time);

void display(void) {}
void mouse(int btn, int state, int x, int y) {}
void mouseMotion(int x, int y) {}
void myReshape(int w, int h);

char Buffer[1024];
int InputFD= 0;

long StartTime;

CmdlineOptions Options= { NULL, false, false, false };

int main(int Argc, char**Args) {
	struct timeval curtime;
	
	ReadParams(Args, &Options);
	
	if (Options.WantHelp || Options.NeedHelp) {
		PrintUsage();
		return Options.WantHelp? 0 : -1;
	}
	
	if (Options.FifoName) {
		if (CreateListenSocket(Options.FifoName, &InputFD))
			close(0);
		else
			return 2;
	}

	DEBUGMSG(("Initializing glut\n"));
	glutInit(&Argc, Args);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("CmdlineGL");

	DEBUGMSG(("Assigning functions\n"));
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	DEBUGMSG(("Setting timer\n"));
	glutTimerFunc(1, WatchSocket, 0);

	DEBUGMSG(("Resizing window\n"));
	glutInitWindowSize(500, 501);

	DEBUGMSG(("Recording time\n"));
	gettimeofday(&curtime, NULL);
	StartTime= microseconds(&curtime);
	
	DEBUGMSG(("Entering glut loop\n"));
	glutMainLoop();
}

void WatchSocket(int id) {
	// Read an process one command
	int result= ProcessFD(InputFD);
	if (result == P_EOF) {
		DEBUGMSG(("Received EOF.\n"));
 		if (TerminateOnEOF)
			Shutdown= 1;
	}
	else
		glutTimerFunc(1, WatchSocket, 0);

	if (Shutdown) {
		DEBUGMSG(("Shutting down.\n"));
		close(InputFD);
		// this might be a socket
		if (SocketName) {
			unlink(SocketName);
		}
		exit(0);
	}
}

void ReadParams(char **Args, CmdlineOptions *Options) {
	char **NextArg= Args+1;
	bool ReadingArgs= true;
	// Note: this function takes advantage of the part of the program argument specification
	//   that says all argv lists must end with a NULL pointer.
	while (ReadingArgs && *NextArg && (*NextArg)[0] == '-') {
		switch ((*NextArg)[1]) {
		case 'f': Options->FifoName= *++NextArg; break; // '-f <blah>' = read from FIFO "blah"
		case 't': Options->TerminateOnEOF= true; break;
		case 'h':
		case '?': Options->WantHelp= true; break;
		case '\0': ReadingArgs= false; break; // "-" = end of arguments
		case '-': // "--" => long-option
			if (strcmp(*NextArg, "--help") == 0) { Options->WantHelp= true; break; }
		default:
			fprintf(stderr, "Unrecognized argument: %s", *NextArg);
			Options->NeedHelp= true;
			return;
		}
		NextArg++;
	}
}

void PrintUsage() {
	fprintf(stderr, "%s",
	"Usage:\n"
	"  CmdlineGL -f <fifoname> [-t] | <input-processor>\n"
	"     Read commands from the named fifo, and write user input to stdout.\n"
	"\n"
	"  <command-stream> | CmdlineGL [-t] | <input-processor>\n"
	"     Read commands from stdin, and write user input to stdout.\n"
	"\n"
	"  CmdlineGL -h\n"
	"     Display this help message.\n"
	"\n"
	"socketname : a path/filename for a unix socket, which the server creates\n"
	"-t         : terminate after receiving EOF\n"
	"\n"
	"\n"
	"Note: Each line of input is broken on space characters and treated as a\n"
 	"      command.  There is currently no escaping mechanism, although I'm not\n"
 	"      opposed to the idea.\n\n");
}

long microseconds(struct timeval *time) {
	return time->tv_usec + ((long) time->tv_sec)*1000000;
}

PUBLISHED(exit,DoQuit) {
	Shutdown= true;
}

PUBLISHED(sync,DoSync) {
	struct timeval curtime;
	long target, t;
	char *endptr;
	
	if (argc != 1) return ERR_PARAMCOUNT;
	target= strtol(argv[0], endptr, 10) * 1000;
	if (endptr != '\0') return ERR_PARAMPARSE;
	
	gettimeofday(&curtime, NULL);
	t= microseconds(&curtime) - StartTime;
	if (target - t > 0)
		usleep(target - t);
}

bool CreateListenSocket(char *SocketName, int *ListenSocket) {
	int sock;
	struct sockaddr_un Addr;

	// build the address
	Addr.sun_family= AF_UNIX;
	if (strlen(SocketName) >= sizeof(Addr.sun_path)) {
		fprintf(stderr, "Socket name too long.");
		return false;
	}
	strcpy(Addr.sun_path, SocketName);

	sock= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("create socket");
		return false;
	}

	if (bind(sock, (struct sockaddr*) &Addr, sizeof(Addr))) {
		perror("bind socket");
		return false;
	}

	*ListenSocket= sock;
}

void myReshape(int w, int h) {
	// Use the entire window for rendering.
	glViewport(0, 0, w, h);
	// Recalculate the projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat top, bottom, left, right;
	if (w<h) {
		// if the width is less than the height, make the viewable width
		// 20 world units wide, and calculate the viewable height assuming
		// as aspect ratio of "1".
		left= -10;
		right= 10;
		bottom= -10.0 * ((GLfloat)h) / w;
		top= 10.0 * ((GLfloat)h) / w;
	}
	else {
		// if the height is less than the width, make the viewable height
		// 20 world units tall, and calculate the viewable width assuming
		// as aspect ratio of "1".
		left= -10.0 * ((GLfloat)w) / h;
		right= 10.0 * ((GLfloat)w) / h;
		bottom= -10;
		top= 10;
	}

	// In perspective mode, use 1/10 the world width|height at the near
	//  clipping plane.
	glFrustum(left/10, right/10, bottom/10, top/10, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
}


