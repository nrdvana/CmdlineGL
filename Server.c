#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "Global.h"
#include "Server.h"
#include "ProcessInput.h"

bool Shutdown= false;

PUBLISHED(quit,DoQuit);
PUBLISHED(exit,DoQuit) {
	Shutdown= true;
}

bool ReadParams(char **args, char **SocketName, bool *TerminateOnEOF);
bool CreateListenSocket(char *SocketName, int *ListenSocket);
void WatchSocket(int id);

void display(void) {}
void mouse(int btn, int state, int x, int y) {}
void mouseMotion(int x, int y) {}
void myReshape(int w, int h);

char Buffer[1024];
int InputFD= 0;
char *SocketName= NULL;
bool TerminateOnEOF= false;

int main(int Argc, char**Args) {
	if (!ReadParams(Args, &SocketName, &TerminateOnEOF))
		return 1;
	
	if (SocketName) {
		if (CreateListenSocket(SocketName, &InputFD))
			close(0);
		else
			return 2;
	}

	DEBUGMSG("Initializing glut\n");
	glutInit(&Argc, Args);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("CmdlineGL");

	DEBUGMSG("Assigning functions\n");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	DEBUGMSG("Setting timer\n");
	glutTimerFunc(1, WatchSocket, 0);

	DEBUGMSG("Resizing window\n");
	glutInitWindowSize(500, 501);

	DEBUGMSG("Entering glut loop\n");
	glutMainLoop();
}

void WatchSocket(int id) {
	// Read an process one command
	int result= ProcessFD(InputFD);
	if (result == P_EOF) {
		DEBUGMSG("Received EOF.\n");
 		if (TerminateOnEOF)
			Shutdown= 1;
	}
	else
		glutTimerFunc(1, WatchSocket, 0);

	if (Shutdown) {
		DEBUGMSG("Shutting down.\n");
		close(InputFD);
		// this might be a socket
		if (SocketName) {
			unlink(SocketName);
		}
		exit(0);
	}
}

bool ReadParams(char **Args, char **SocketNameResult, bool *TerminateOnEOFResult) {
	char **NextArg= Args+1;
	bool ReadingArgs= true;
	// Note: this function takes advantage of the part of the program argument specification
	//   that says all argv lists must end with a NULL pointer.
	while (ReadingArgs && *NextArg && (*NextArg)[0] == '-') {
		switch ((*NextArg)[1]) {
		case 'l': *SocketNameResult= *++NextArg; break; // '-l BLAH' = listen on socket BLAH
		case 't': *TerminateOnEOFResult= true; break;
		case '\0': ReadingArgs= false; break; // "-" = end of arguments
		case '-': // "--" => long-option
		default:
			fprintf(stderr, "Unrecognized argument: %s", *NextArg);
			return false;
		}
		NextArg++;
	}
	return true;
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


