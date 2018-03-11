#define INCLUDE_GL
#define INCLUDE_SDL
#include <config.h>
#include "Global.h"
#include "Version.h"
#include "Server.h"
#include "ProcessInput.h"
#include "SymbolHash.h"
#include "ParseGL.h"

bool Shutdown= false;

typedef struct {
	char *FifoName;
	bool TerminateOnEOF;
	bool ShowCmds, ShowConsts;
	bool WantHelp;
	bool NeedHelp;
	bool VersionOnly;
	char *WndTitle;
	bool NoEmitEvents;
	bool ManualSDLSetup;
	bool ManualViewport;
	bool ManualProjection;
	int Width;
	int Height;
	int Bpp;
} CmdlineOptions;

void SetParamDefaults(CmdlineOptions *Options);
void ReadParams(char **args, CmdlineOptions *Options);
void PrintUsage(bool error);
void CheckInput();
void CheckSDLEvents();
void InitGL(int, int);
void EmitKey(const SDL_KeyboardEvent *E);

bool PendingResize= false;
int Resize_w, Resize_h;
void FinishResize();

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
	else if (isatty(InputFD)) {
		fprintf(stderr, "Warning: STDIN is a terminal; will use blocking reads\n");
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

		SDL_EnableUNICODE(1);
		if (SDL_EnableKeyRepeat(0, 0) < 0)
			fprintf(stderr, "Can't disable key repeat. %s\n", SDL_GetError());

		InitGL(Options.Width, Options.Height);

		if (!Options.WndTitle) Options.WndTitle= "CmdlineGL";
		SDL_WM_SetCaption(Options.WndTitle, Options.WndTitle);
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
			if (strcmp(*NextArg, "--noevents") == 0) { Options->NoEmitEvents= true; break; }
			if (strcmp(*NextArg, "--manual-viewport") == 0) { Options->ManualViewport= true; break; }
			if (strcmp(*NextArg, "--manual-projection") == 0) { Options->ManualProjection= true; break; }
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
			fprintf(stderr, "Unrecognized argument: %s\n", *NextArg);
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
	"  -t                  Terminate after a zero-length read (EOF).\n"
	"  --manual-viewport   No automatic glViewport on window resize\n"
	"  --manual-projection No default GL_PROJECTION matrix\n"
	#ifndef WIN32
	"  -f <fifo>           Create the named fifo (file path+name) and read from it.\n"
	#endif
	"  --title <text>      Set the title of the window to \"text\".\n"
	"  --noevents          Don't print any input event messages to stdout.\n"
	"  --showcmds          List all the available commands in this version of CmdlineGL.\n"
	"  --showconsts        List all the constants (GL_xxxx) that are available.\n"
	"  -v --version        Print version and exit.\n"
	"  -h                  Display this help message.\n"
	"\n"
	"Note: Each line of input is broken on space characters and treated as a\n"
 	"      command.  There is currently no escaping mechanism, although I'm not\n"
 	"      opposed to the idea.\n\n",
	CGLVER_String
	);
}

/*
=head2 Render Loop Commands

=over

=item cglEcho ANY_TEXT

Repeat a string of text on stdout (may be confused for user input events,
but maybe that's what you want)

=cut
*/
COMMAND(cglEcho, "b") {
	printf("%s\n", parsed->strings[0]);
	fflush(stdout);
	return true;
}

/*
=item cglExit

Cause CmdlineGL to terminate (with error code 0)

=item cglQuit

Alias for cglExit

=cut
*/
COMMAND(cglExit, "") {
	Shutdown= true;
	return true;
}
COMMAND(cglQuit, "") {
	Shutdown= true;
	return true;
}

/*
=item cglGetTime

Return the number of milliseconds since start of the program.

=item cglSleep DELAY_MS

Sleep for a number of milliseconds from when the command is received

=item cglSync UNTIL_TIME_MS

Sleep until the specified time, measured as milliseconds from start of program.

=cut
*/
COMMAND(cglGetTime, "") {
	long t= SDL_GetTicks() - StartTime;
	printf("t=%ld\n", t);
	fflush(stdout);
	return true;
}

COMMAND(cglSleep, "i") {
	fflush(stdout);
	fflush(stderr);
	SDL_Delay(parsed->ints[0]);
	return true;
}

COMMAND(cglSync, "i") {
	int target= parsed->ints[0];
	int t= SDL_GetTicks() - StartTime;
	if (target - t > 0) {
		fprintf(stderr, "sleeping for %lld\n", target - t);
		fflush(stdout);
		fflush(stderr);
		SDL_Delay(target - t);
	}
	return true;
}

/*
=item cglSwapBuffers

Swap front and back buffer, showing the frame you were drawing and beginning
a new frame.  (you still need to call glClear yourself)
If a window resize is pending, it will be performed at this point.

=back

=cut
*/
COMMAND(cglSwapBuffers, "") {
	SDL_GL_SwapBuffers();
	FrameInProgress= false;
	// If we were waiting to resize the window, now is the time
	if (PendingResize) FinishResize();
	return true;
}

void CheckInput() {
	int result, TokenCount, CmdCount;
	char *Line, *TokenPointers[MAX_GL_PARAMS+1];

	errno= 0;
	CmdCount= 0;
	while ((CmdCount < MAX_COMMAND_BATCH || PointsInProgress) && (Line= ReadLine())) {
		if (*Line && *Line != '#') {
			//fprintf(stderr, "Starting '%s'\n", Line);
			ProcessCommand(Line);
			//fprintf(stderr, "Ending '%s'\n", Line);
			CmdCount++;
		}
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
				if (!Options.NoEmitEvents)
					EmitKey(&Event.key);
				break;
			case SDL_MOUSEMOTION:
				if (!Options.NoEmitEvents)
					printf("M @ %d %d %d %d\n", Event.motion.x, Event.motion.y,
						Event.motion.xrel, Event.motion.yrel);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (!Options.NoEmitEvents)
					printf("M %c %d %d %d\n", (Event.button.state == SDL_PRESSED)? '+':'-',
						Event.button.button, Event.button.x, Event.button.y);
				break;
			case SDL_VIDEORESIZE:
				Resize_w= Event.resize.w;
				Resize_h= Event.resize.h;
				// Can't resize if user is in the middle of rendering
				if (FrameInProgress)
					PendingResize= true;
				else
					FinishResize();
				break;
			case SDL_ACTIVEEVENT:
				if (!Options.NoEmitEvents) {
					if (Event.active.state & SDL_APPMOUSEFOCUS)
						printf("W MOUSEFOCUS %c\n", Event.active.gain? '+':'-');
					if (Event.active.state & SDL_APPINPUTFOCUS)
						printf("W INPUTFOCUS %c\n", Event.active.gain? '+':'-');
					if (Event.active.state & SDL_APPACTIVE)
						printf("W ACTIVE %c\n", Event.active.gain? '+':'-');
				}
				break;
			case SDL_QUIT:
				if (!Options.NoEmitEvents)
					printf("W QUIT\n");
				Shutdown= true;
				break;
		}
	if (!Options.NoEmitEvents)
		fflush(stdout);
}

// This should only be called when FrameInProgress is false
// Might get called from ParseGL.c, so args are stored in globals.
void FinishResize() {
	assert(!FrameInProgress);
	MainWnd= SDL_SetVideoMode(Resize_w, Resize_h, Options.Bpp, DefaultSDLFlags);
	if (!Options.NoEmitEvents) {
		printf("W RESIZE %d %d\n", Resize_w, Resize_h);
		fflush(stdout);
	}
	PendingResize= false;
	InitGL(Resize_w, Resize_h);
}

void InitGL(int w, int h) {
	GLfloat top, bottom, left, right;
	GLfloat dist= 20; // 20 units from near clipping plane to the origin
	assert(!FrameInProgress);

	if (!Options.ManualViewport)
		// Use the entire window
		glViewport(0, 0, w, h);

	if (!Options.ManualProjection) {
		// Recalculate the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (w<h) {
			// if the width is less than the height, make the viewable width
			// 20 world units wide, and calculate the viewable height assuming
			// an aspect ratio of "1".
			left= -10;
			right= 10;
			bottom= -10.0 * ((GLfloat)h) / w;
			top= 10.0 * ((GLfloat)h) / w;
		}
		else {
			// if the height is less than the width, make the viewable height
			// 20 world units tall, and calculate the viewable width assuming
			// an aspect ratio of "1".
			left= -10.0 * ((GLfloat)w) / h;
			right= 10.0 * ((GLfloat)w) / h;
			bottom= -10;
			top= 10;
		}

		glFrustum(left/dist, right/dist, bottom/dist, top/dist, 1, 1000);
		glTranslated(0, 0, -dist);
		glMatrixMode(GL_MODELVIEW);
	}
}

void encode_utf8(char *buffer, unsigned codepoint);
void EmitKey(const SDL_KeyboardEvent *E) {
	char kname[64], uni[5], *c;
	unsigned int codepoint= E->keysym.unicode;
	strncpy(kname, SDL_GetKeyName(E->keysym.sym), sizeof(kname));
	kname[sizeof(kname)-1]= '\0';
	// remove any whitespace in the key name
	for (c= kname; *c; c++)
		if (*c <= ' ') *c= '_';
	// If unicode available, encode as utf8
	if (codepoint > (unsigned) ' ')
		encode_utf8(uni, codepoint);
	else
		uni[0]= 0;
	printf("K %c %s %d %04x %s\n", (E->state == SDL_PRESSED)? '+':'-', kname,
		E->keysym.scancode, codepoint, uni);
}

// Yeah this exists elsewhere but I don't feel like depending on a lib for just this one function
void encode_utf8(char *buffer, unsigned codepoint) {
	if (codepoint < 0x80)
		*buffer++= (char) codepoint;
	else if (codepoint < 0x800) {
		*buffer++= (char) 0xC0 | ((codepoint >> 6) & 0x1F);
		*buffer++= (char) 0x80 | ((codepoint     ) & 0x3F);
	}
	else if (codepoint < 0x10000) {
		*buffer++= (char) 0xE0 | ((codepoint >> 12) & 0x0F);
		*buffer++= (char) 0x80 | ((codepoint >>  6) & 0x3F);
		*buffer++= (char) 0x80 | ((codepoint      ) & 0x3F);
	}
	else if (codepoint < 0x110000) {
		*buffer++= (char) 0xF0 | ((codepoint >> 18) & 0x07);
		*buffer++= (char) 0x80 | ((codepoint >> 12) & 0x3F);
		*buffer++= (char) 0x80 | ((codepoint >>  6) & 0x3F);
		*buffer++= (char) 0x80 | ((codepoint      ) & 0x3F);
	}
	*buffer= 0;
}
