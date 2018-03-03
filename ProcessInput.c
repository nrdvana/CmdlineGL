#include "Global.h"
#include "GlHeaders.h"

#include <assert.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include "ProcessInput.h"
#include "ParseGL.h"
#include "SymbolHash.h"

#define READ_BUFFER_SIZE 1024

/* CmdlineGL supports fixed-point numbers on stdin, by multiplying
 * all float values by this multiplier.  Naturally, it defaults to 1.0
 */
double FixedPtMultiplier= 1.0;
double DivisorStack[16];
int DivisorStackPos= -1;
const int DivisorStackMax= sizeof(DivisorStack)/sizeof(double) - 1;

/*
=item cglPushDivisor DIVISOR

All future floating-point numbers should have an implied "/DIVISOR" on them.
This does not replace a divisor that is manually specified.

For example,

    cglPushDivisor 100
    glRotate 12/10000 100 100 100

becomes

    glRotated(.0012, 1, 1, 1);

=item cglPopDivisor

Clear the previous divisor, returning to whatever default was set before that.
The initial default is 1.

=cut
*/
COMMAND(cglPushDivisor, "s") {
	char *EndPtr;
	double newval;
	if (DivisorStackPos >= DivisorStackMax) {
		fprintf(stderr, "cglPushDivisor: stack overflow");
		return false;
	}
	// We don't want to scale the new scale... so don't use ParseParams("f").
	newval= strtod(argv[0].as_str, &EndPtr);
	if (*EndPtr != '\0') {
		fprintf(stderr, "cglPushDivisor: expected Float");
		return false;
	}
	DivisorStack[++DivisorStackPos]= newval;
	FixedPtMultiplier= 1.0 / newval;
	return true;
}

COMMAND(cglPopDivisor, "") {
	if (DivisorStackPos < 0)  {
		fprintf(stderr, "cglPpopDivisor: stack underflow");
		return false;
	}
	FixedPtMultiplier= (--DivisorStackPos >= 0)? 1.0 / DivisorStack[DivisorStackPos] : 1.0;
	return true;
}

/* Below is "class LineBuffer".
 * I had to write my own because I wanted a readline function that wouldn't
 * block, and wouldn't return a partial string.
 *
 * There's only one used in the whole prog, and I see it staying that way, so I
 * 'optimized' a bit by making it global.
 *
 * If it ever becomes necessary to make more than one, uncomment and propogate.
 * Also, it will then be necessary to make a constructor function, and call it
 * before each gets used the first time.
 */
//typedef struct LineBuffer_t {
	char ReadBuffer[READ_BUFFER_SIZE];
	char const *StopPos;
	char *Pos;
	char *DataPos;
	char *LineStart;
	#ifndef _WIN32
	int fd;
	#else
	HANDLE fd;
	#endif
//} LineBuffer;

#ifndef _WIN32
void InitLineBuffer(int FileHandle /*, LineBuffer *this */) {
#else
void InitLineBuffer(HANDLE FileHandle /*, LineBuffer *this */) {
#endif
	fd= FileHandle;
	StopPos= ReadBuffer+READ_BUFFER_SIZE;
	Pos= ReadBuffer;
	DataPos= ReadBuffer;
	LineStart= ReadBuffer;
}

void ShiftBuffer(/* LineBuffer *this */) {
	int shift= LineStart - ReadBuffer;
	if (DataPos != LineStart)
		memmove(ReadBuffer, LineStart, DataPos - LineStart);
	DataPos-= shift;
	Pos-= shift;
	LineStart= ReadBuffer;
}

char* ReadLine(/* LineBuffer *this */) {
	int success;
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
			#ifndef _WIN32
			red= read(fd, DataPos, StopPos-DataPos);
			if (red <= 0)
				return NULL;
			#else
			// no non-blocking mode, so we need to test the status of the object
			if (WaitForSingleObject(fd, 0) == WAIT_OBJECT_0)
				success= ReadFile(fd, DataPos, StopPos-DataPos, &red, NULL);
			else
				return NULL;
			if (!success || red<=0)
				return NULL;
			#endif
			DataPos+= red;
		}
		while (Pos < DataPos && *Pos != '\n')
			Pos++;
	} while (*Pos != '\n');
	*Pos= '\0';
	if (Pos > LineStart)
		if (Pos[-1] == '\r')
			Pos[-1]= 0;
	Result= LineStart;
	LineStart= ++Pos;
	return Result;
}

char * next_token(char **input) {
	char q, *out, *in= input? *input : NULL;
	if (!in) return NULL;
	/* skip delim characters */
	while (*in == ' ' || *in == '\t') in++;
	/* quoting support */
	out= in;
	if (*in == '\'' || *in == '"') {
		q= *in++;
		while (*in != '\0' && *in != q) {
			if (*in == '\\') {
				switch (* ++in) {
				case 'n': *in= '\n'; break;
				case 'r': *in= '\r'; break;
				case '\0': --in; break; /* don't run over end of string */
				default: 0; /* every other value remains itself */
				}
			}
			*out++ = *in++;
		}
		*out= '\0';
	}
	/* else just look for end of token */
	else {
		while (*in && *in != ' ' && *in != '\t') in++;
		*in= '\0';
		if (!*out) out= NULL; /* return null unless found a token */
	}
	while (*in == ' ' || *in == '\t') in++;
	*input= in; /* give caller back the modified pointer */
	return out;
}

char *sanitize_for_print(char *string) {
	for (char *c= string; *c; c++)
		if (*c <= ' ' || *c == 0x7F)
			*c= '_';
	return string;
}

/* ProcessCommand bites off the first word and dispatches to a command of that name.
 * It destroys "Line" with strtok in the process.
 */
int ProcessCommand(char *Line) {
	const CmdHashEntry *Cmd;
	ParamList params;
	int GLErr, n_params;
	
	char *command= next_token(&Line);
	// Ignore blank lines or comments
	if (!command || !*command || *command == '#')
		return P_IGNORE;
	
	Cmd= GetCmd(command);
	sanitize_for_print(command);
	if (!Cmd) {
		fprintf(stderr, "Unknown command: \"%s\".\n", command);
		return P_NOT_FOUND;
	}
	
	/* use command's format string to parse each argument to the correct type */
	if (!ParseParams(command, Line, Cmd->ArgFormat, params, &n_params))
		return P_CMD_ERR;
	/* dispatch command */
	if (!Cmd->Handler(n_params, params)) {
		fprintf(stderr, "Error during execution of %s\n", command);
		return P_CMD_ERR;
	}
	
	/* Check GL status, unless in the middle of begin/end */
	if (!PointsInProgress) {
		GLErr= glGetError();
		if (GLErr) { // command was successful
			while (GLErr) {
				fprintf(stderr, "GL error while executing %s: %s.\n", command, gluErrorString(GLErr));
				GLErr= glGetError();
			}
			return P_CMD_ERR;
		}
	}

	return P_SUCCESS;
}

SymbVarEntry* CreateNamedObj(const char* Name, int Type);

bool ParseParams(char *command, char *line, const char *format, ParamUnion *params, int *n_params) {
	char errbuf[128];
	if (!ParseParamsCapErr(line, format, params, n_params, errbuf, sizeof(errbuf))) {
		fprintf(stderr, "Error parsing params of %s: %s", command, errbuf);
		return false;
	}
	return true;
}

bool ParseParamsCapErr(char *line, const char *format, ParamUnion *params, int *n_params, char* errbuf, int errbufsize) {
	const char *tok= "", *err= NULL, *err1= NULL;
	ParamUnion *ppos= params, *plim= params + *n_params;
	float cfloat[4];
	int consumed, objtyp;
	
	while (*format && *line && ppos < plim) {
		consumed= 1; // default
		switch (*format) {
		// Integer value
		case 'i':
		case 'l': if (!ParseLong(tok= next_token(&line), &ppos->as_long)) err= "expected Integer"; break;
		// Floating value
		case 'f':
		case 'd': if (!ParseDouble(tok= next_token(&line), &ppos->as_double)) err= "expected Float"; break;
		// single token
		case 't': if (!(tok= next_token(&line))) err= "expected Token"; break;
		// Display list
		case 'L': objtyp= NAMED_LIST; if (0)
		// Quadric
		case 'Q': objtyp= NAMED_QUADRIC; if (0)
		// Texture
		case 'T': objtyp= NAMED_TEXTURE; if (0)
		// Font
		case 'F': objtyp= NAMED_FONT;
			tok= next_token(&line);
			if (format[1] == '!') /* auto-create */
				++format;
			if (!ParseSymbVar(tok, objtyp, format[0] == '!', &ppos->as_sym)) {
				err= "Named %s '%s' does not exist";
				err1= SymbVarTypeName[objtyp];
			}
			break;
		// Color, either "#xxxxxx" or 3xFloat or 4xFloat
		case 'c':
			tok= line;
			if (!ParseColor(&line, ppos->as_color))
				err= "expected Color";
			break;
		// remainder of buffer
		case 'b':
			tok= line;
			line += strlen(line);
			break;
		// string, either one token or rest of line if quoted
		case 's':
		// Stat a path
		case '/':
			if (*line == '"') tok= next_token(&line);
			else {
				tok= line;
				line += strlen(line);
			}
			if (*format == '/' && !FileExists(tok)) {
				err= "no such file '%s'";
				err1= tok;
			}
			break;
		default:
			assert(0 == "all formats handled");
			return false;
		}
		
		if (err) {
			// allow parse failure to move past '*' or '?', unless it is the end.
			if ((format[1] == '*' || format[1] == '?') && format[2] != '\0') {
				format += 2;
				err= NULL;
			}
			else
				break;
		}
		else {
			++ppos;
			// Advance format, unless it is a repeating element
			if (format[1] != '*')
				format += format[1] == '?'? 2 : 1;
			// but if repeating an auto-create, back up to the actual format code
			else if (format[0] == '!')
				--format;
		}
	}
	// Check for argument count errors
	if (!err) {
		if (ppos == plim)
			err= "exceeded max number of arguments";
		else if (*line)
			err= "unpexpected extra arguments";
		else {
			/* skip past optional args to see if we missed a mandatory one */
			while (*format && !err) {
				if (format[1] == '*' || format[1] == '?')
					format+= 2;
				else if (format[1] == '!')
					format++;
				else
					err= "not enough arguments";
			}
		}
	}
	// If error, format it into the caller's buffer
	if (err) {
		snprintf(errbuf, errbufsize, err, err1, tok);
		return false;
	}
	*n_params= ppos - params;
	return true;
}

bool ParseLong(const char* Text, long *Result) {
	const IntConstHashEntry *SymConst;
	char *EndPtr;
	
	if ((Text[0] == 'G' && Text[1] == 'L')
		|| (Text[0] == 'C' && Text[1] == 'G' && Text[2] == 'L'))
	{
		DEBUGMSG(("Searching for %s...", Text));
		SymConst= GetIntConst(Text);
		if (SymConst) {
			DEBUGMSG((" Found: %i\n", SymConst->Value));
			*Result= SymConst->Value;
			return true;
		}
		else {
			DEBUGMSG(("not found.\n"));
			return false;
		}
	}
	else if (Text[0] == '0' && Text[1] == 'x') {
		*Result= strtol(Text, &EndPtr, 16);
		return (*EndPtr == '\0');
	}
	else {
		*Result= strtol(Text, &EndPtr, 10);
		DEBUGMSG((*EndPtr == '\0'? "" : "%s is not a constant.\n", Text));
		return (*EndPtr == '\0');
	}
}

bool ParseDouble(const char* Text, double *Result) {
	char *EndPtr;
	double num, denom;
	if (Text[0] == '-' && Text[1] == '-') Text+= 2; // be nice about double negatives
	num= strtod(Text, &EndPtr);
	if (*EndPtr == '/') {
		denom= strtod(EndPtr+1, &EndPtr);
		if (!denom) return 0;
		*Result= num / denom;
	} else {
		*Result= num * FixedPtMultiplier;
	}
	return (*EndPtr == '\0');
}
bool ParseFloat(const char *Text, float *Result) {
	double x;
	if (!ParseDouble(Text, &x)) return false;
	*Result= x;
	return true;
}

bool ParseColor(char **line, float color[4]) {
	double d;
	int n;
	
	if (!line || !*line || !**line)
		return false;
	// hash indicates a hex code
	if (**line == '#') {
		return ParseHexColor(next_token(line), color);
	}
	else {
		// else look for 3 or 4 numbers
		for (n= 0; n < 4; n++) {
			/* avoid calling next_token unless it does look like a valid number */
			if (!((**line >= '0' && **line <= '9') || **line == '+' || **line == '-'))
				break;
			if (!ParseDouble(next_token(line), &d))
				break;
			color[n]= (float) d;
		}
		if (n < 3) return false;
		if (n < 4) color[3]= 1.0;
	}
	return true;
}
bool ParseHexByte(const char *str, float *Result);
bool ParseHexColor(const char* Text, float Result[4]) {
	if (!Text || Text[0] != '#') return false;
	// could use strtol instead of this, but then would have to worry about endian issues,
	// made more awkward by presence or absence of Alpha channel
	if (!ParseHexByte(Text+1, Result+0)) return false;
	if (!ParseHexByte(Text+3, Result+1)) return false;
	if (!ParseHexByte(Text+5, Result+2)) return false;
	if (Text[7]) {
		if (!ParseHexByte(Text+7, Result+3)) return false;
	} else {
		Result[3]= 0xFF;
	}
}
bool ParseHexByte(const char *str, float *Result) {
	GLubyte x;
	char h= str[0], l= str[1];
	if      (h >= '0' && h <= '9') x= h -  '0';
	else if (h >= 'A' && h <= 'F') x= h - ('A' - 10);
	else if (h >= 'a' && h <= 'f') x= h - ('a' - 10);
	else return false;
	x <<= 4;
	if      (l >= '0' && l <= '9') x|= l -  '0';
	else if (l >= 'A' && l <= 'F') x|= l - ('A' - 10);
	else if (l >= 'a' && l <= 'f') x|= l - ('a' - 10);
	else return false;
	*Result= x * 1.0/255;
	return true;
}

bool ParseSymbVar(const char* Text, int Type, bool autocreate, SymbVarEntry **Result) {
	SymbVarEntry *Entry= GetSymbVar(Text, Type);
	if (Entry) {
		*Result= Entry;
		return true;
	}
	if (!autocreate || !*Text) /* need a non-empty name, for autocreate */
		return false;
	/* We can auto-create every object type except fonts.  For fonts, return a NULL pointer */
	if (Type == NAMED_FONT)
		Entry= NULL;
	else {
		Entry= CreateSymbVar(Text, Type); // entry is added to RB tree at this point
		switch (Type) {
		case NAMED_LIST: Entry->Value= glGenLists(1); break;
		case NAMED_QUADRIC: Entry->Data= gluNewQuadric(); break;
		case NAMED_TEXTURE: glGenTextures(1, &(Entry->Value)); break;
		default: Entry->Data= NULL; break;
		}
	}
	*Result= Entry;
	return true;
}

bool FileExists(const char *Name) {
#ifdef _WIN32
	return PathFileExists(Name);
#else
	struct stat s;
	return stat(Name, &s) == 0;
#endif
}
