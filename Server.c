#include "Global.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "GlHeaders.h"
#include <SDL/SDL.h>
#include "Version.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Server.h"
#include "ProcessInput.h"

bool Shutdown= false;

typedef struct {
	char *FifoName;
	bool TerminateOnEOF;
	bool ShowCmds, ShowConsts;
	bool WantHelp;
	bool NeedHelp;
	bool VersionOnly;
	char *WndTitle;
	bool NoUIMessages;
	bool ManualSDLSetup;
	bool ManualGLSetup;
	bool ManualResizeCode;
	int Width;
	int Height;
	int Bpp;
} CmdlineOptions;

void SetParamDefaults(CmdlineOptions *Options);
void ReadParams(char **args, CmdlineOptions *Options);
void PrintUsage(bool error);
void CheckInput();
void CheckSDLEvents();

void InitGL();
void HandleResize(int w, int h);
void EmitMouseMotion(const SDL_MouseMotionEvent *E);
void EmitMouseButton(const SDL_MouseButtonEvent *E);
void EmitKey(const SDL_KeyboardEvent *E);

char Buffer[1024];
#ifndef _WIN32
int InputFD= 0;
#else
HANDLE InputFD;
#endif
SDL_Surface *MainWnd= NULL;

long StartTime;
int DefaultSDLFlags= SDL_OPENGL | SDL_ANYFORMAT | SDL_RESIZABLE;

CmdlineOptions Options;

int main(int Argc, char**Args) {
	const SDL_VideoInfo* SdlVid;
	int bpp;

	SetParamDefaults(&Options);
	ReadParams(Args, &Options);
	
	if (Options.WantHelp || Options.NeedHelp) {
		PrintUsage(!Options.WantHelp);
		return Options.WantHelp? 0 : -1;
	}
	if (Options.VersionOnly) {
		printf("Version %s\n", CGLVER_String);
		return 0;
	}
	if (Options.ShowCmds) {
		DumpCommandList(stdout);
		return 0;
	}
	if (Options.ShowConsts) {
		DumpConstList(stdout);
		return 0;
	}

	#ifndef WIN32
	if (Options.FifoName) {
		if (mkfifo(Options.FifoName, 0x1FF) < 0) {
			perror("mkfifo");
			return 2;
		}
		InputFD= open(Options.FifoName, O_RDONLY | O_NONBLOCK);
		if (InputFD < 0) {
			perror("open(fifo, read|nonblock)");
			return 3;
		}
		close(0); // Done with stdin.
	}
	else {
		DEBUGMSG(("Enabling nonblocking mode on stdin\n"));
		if (fcntl(InputFD, F_SETFL, O_NONBLOCK) < 0) {
			perror("fnctl(stdin, F_SETFL, O_NONBLOCK)");
			return 3;
		}
	}
	#else
	InputFD= GetStdHandle(STD_INPUT_HANDLE);
	#endif
	InitLineBuffer(InputFD); // this initializes a static global LineBuffer.  See ProcessInput.c

	if (!Options.ManualSDLSetup) {
		DEBUGMSG(("Initializing SDL\n"));
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			fprintf(stderr, "CmdlineGL: SDL_Init: %s\n", SDL_GetError());
			return 4;
		}
		atexit(SDL_Quit);

//		SdlVid= SDL_GetVideoInfo();
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		DEBUGMSG(("Setting video mode\n"));
		MainWnd= SDL_SetVideoMode(Options.Width, Options.Height, Options.Bpp, DefaultSDLFlags);
		if (!MainWnd) {
			fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
			exit(5);
		}

		if (SDL_EnableKeyRepeat(0, 0) < 0)
			fprintf(stderr, "Can't disable key repeat. %s\n", SDL_GetError());

		if (!Options.ManualGLSetup)
			InitGL(Options.Width, Options.Height);
	}

	DEBUGMSG(("Recording time\n"));
	StartTime= SDL_GetTicks();

	DEBUGMSG(("Entering main loop\n"));
	while (!Shutdown) {
		CheckInput();
		CheckSDLEvents();
		SDL_Delay(1); // yield processor to the script
	}
	DEBUGMSG(("Shutting down.\n"));

	#ifndef _WIN32
	close(InputFD);
	#else
	CloseHandle(InputFD);
	#endif
	// this might be a socket
	if (Options.FifoName) {
		unlink(Options.FifoName);
	}

	return 0;
}

void SetParamDefaults(CmdlineOptions *Options) {
	memset(Options, 0, sizeof(CmdlineOptions));
	Options->Width= 640;
	Options->Height= 480;
	// Options->Bpp= 0; zero tells to use display default
}

void PrintMissingParamMessage(char* ArgName, CmdlineOptions *Options) {
	fprintf(stderr, "Option %s requires a parameter\n", ArgName);
	Options->NeedHelp= true;
}

void ReadParams(char **Args, CmdlineOptions *Options) {
	char **NextArg= Args+1;
	bool ReadingArgs= true;
	// Note: this function takes advantage of the part of the program argument specification
	//   that says all argv lists must end with a NULL pointer.
	while (ReadingArgs && *NextArg && (*NextArg)[0] == '-') {
		switch ((*NextArg)[1]) {
		#ifndef WIN32
		case 'f':
			Options->FifoName= *++NextArg;
			if (!Options->FifoName) {
				PrintMissingParamMessage("-f", Options);
				return;
			}
			break; // '-f <blah>' = read from FIFO "blah"
		#endif
		case 't': Options->TerminateOnEOF= true; break;
		case 'v': Options->VersionOnly= true; break;
		case 'h':
		case '?': Options->WantHelp= true; break;
		case '\0': ReadingArgs= false; break; // "-" = end of arguments
		case '-': // "--" => long-option
			if (strcmp(*NextArg, "--help") == 0) { Options->WantHelp= true; break; }
			if (strcmp(*NextArg, "--showcmds") == 0) { Options->ShowCmds= true; break; }
			if (strcmp(*NextArg, "--showconsts") == 0) { Options->ShowConsts= true; break; }
			if (strcmp(*NextArg, "--nouimsg") == 0) { Options->NoUIMessages= true; break; }
			if (strcmp(*NextArg, "--title") == 0) {
				Options->WndTitle= *++NextArg;
				if (!Options->WndTitle) {
					PrintMissingParamMessage("--title", Options);
					return;
				}
				break;
			}
			if (strcmp(*NextArg, "--version") == 0) { Options->VersionOnly= true; break; }
		default:
			fprintf(stderr, "Unrecognized argument: %s", *NextArg);
			Options->NeedHelp= true;
			return;
		}
		NextArg++;
	}
}

void PrintUsage(bool error) {
	fprintf(error? stderr : stdout,
	"CmdlineGL %s\n"
	"Usage:\n"
	"  <command-source> | CmdlineGL [options] | <user-input-reader>\n"
	"     Reads commands from stdin, and writes user input to stdout.\n"
	"\n"
	"Options:\n"
	"  -h              Display this help message.\n"
	"  -t              Terminate after receiving EOF.\n"
	"  -v --version    Display version and exit.\n"
	#ifndef WIN32
	"  -f <fifo>       Create the named fifo (file path+name) and read from it.\n"
	#endif
	"  --showcmds      List all the available commands in this version of CmdlineGL.\n"
	"  --showconsts    List all the constants (GL_xxxx) that are available.\n"
	"  --title <text>  Set the title of the window to \"text\".\n"
	"  --nouimsg       Don't print any user activity messages to stdout.\n"
	"\n"
	"Note: Each line of input is broken on space characters and treated as a\n"
 	"      command.  There is currently no escaping mechanism, although I'm not\n"
 	"      opposed to the idea.\n\n",
	CGLVER_String
	);
}

PUBLISHED(cglExit,DoQuit) {
	Shutdown= true;
	return 0;
}

PUBLISHED(cglGetTime,DoGetTime) {
	long t;
	if (argc != 0) return ERR_PARAMCOUNT;
	t= SDL_GetTicks() - StartTime;
	printf("t=%d\n", t);
	fflush(stdout);
	return 0;
}

PUBLISHED(cglSync,DoSync) {
	long target, t;
	char *endptr;
	
	if (argc != 1) return ERR_PARAMCOUNT;
	target= strtol(argv[0], &endptr, 10);
	if (*endptr != '\0') return ERR_PARAMPARSE;
	
	t= SDL_GetTicks() - StartTime;
	if (target - t > 0)
		SDL_Delay(target - t);
	return 0;
}

PUBLISHED(cglSleep, DoSleep) {
	long t;
	char *endptr;

	if (argc != 1) return ERR_PARAMCOUNT;
	t= strtol(argv[0], &endptr, 10);
	if (*endptr != '\0' || t <= 0) return ERR_PARAMPARSE;
	SDL_Delay(t);
	return 0;
}

PUBLISHED(cglEcho,DoEcho) {
	int i;
	if (argc == 0)
		printf("\n");
	else {
		for (i=0; i<argc-1; i++)
			printf("%s ", argv[i]);
		printf("%s\n", argv[argc-1]);
	}
	fflush(stdout);
	return 0;
}

void CheckInput() {
	int result, TokenCount, CmdCount;
	char *Line, *TokenPointers[MAX_GL_PARAMS+1];

	errno= 0;
	CmdCount= 0;
	while ((CmdCount < MAX_COMMAND_BATCH || IsGlBegun) && (Line= ReadLine())) {
		DEBUGMSG(("%s\n", Line));
		if (ParseLine(Line, &TokenCount, TokenPointers))
			ProcessCommand(TokenPointers, TokenCount);
		else
			DEBUGMSG(("Empty line ignored\n"));
		CmdCount++;
	}
	if (CmdCount < MAX_COMMAND_BATCH && errno != EAGAIN) {
		DEBUGMSG(("Received EOF.\n"));
		if (Options.TerminateOnEOF)
			Shutdown= 1;
	}
}

void CheckSDLEvents() {
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
		switch (Event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				EmitKey(&Event.key);
				break;
			case SDL_MOUSEMOTION:
				EmitMouseMotion(&Event.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				EmitMouseButton(&Event.button);
				break;
			case SDL_VIDEORESIZE:
				HandleResize(Event.resize.w, Event.resize.h);
				break;
			case SDL_QUIT:
				Shutdown= true;
				break;
		}
}

void InitGL(int w, int h) {
	GLfloat top, bottom, left, right;

	// Use the entire window
	glViewport(0, 0, w, h);
	// Recalculate the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

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

void HandleResize(int w, int h) {
	if (!Options.ManualResizeCode) {
		MainWnd= SDL_SetVideoMode(w, h, Options.Bpp, SDL_OPENGL | SDL_ANYFORMAT);
		InitGL(w, h);
	}
	else {
		printf("RESIZE %d %d\n", w, h);
		fflush(stdout);
	}
}

void EmitMouseMotion(const SDL_MouseMotionEvent *E) {
	printf("M @ %d %d %d %d\n", E->x, E->y, E->xrel, E->yrel);
	fflush(stdout);
}

void EmitMouseButton(const SDL_MouseButtonEvent *E) {
	printf("M %c %d %d %d\n", (E->state == SDL_PRESSED)? '+':'-', E->button, E->x, E->y);
	fflush(stdout);
}

void EmitKey(const SDL_KeyboardEvent *E) {
	printf("K %c %s\n", (E->state == SDL_PRESSED)? '+':'-', SDL_GetKeyName(E->keysym.sym));
	fflush(stdout);
}

