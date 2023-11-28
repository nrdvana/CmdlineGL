#define INCLUDE_GL
#include <config.h>

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

/*=head2 Default-Divisor Commands

=item cglPushDivisor DIVISOR

All future floating-point numbers receive this implied "/DIVISOR".
This does not replace a divisor that is manually specified.

For example,

    cglPushDivisor 100
    glRotate 12/1 100 100 100

becomes

    glRotated(12, 1, 1, 1);

=item cglPopDivisor

Clear the current default  divisor, returning to whatever default
was set before that.  The initial default is 1.

=cut*/

COMMAND(cglPushDivisor, "t") {
	char *EndPtr;
	double newval;
	if (DivisorStackPos >= DivisorStackMax) {
		parsed->errmsg= "divisor stack overflow";
		return false;
	}
	// We don't want to scale the new scale... so don't use ParseParams("d").
	newval= strtod(parsed->strings[0], &EndPtr);
	if (*EndPtr != '\0') {
		parsed->errmsg= "expected Float";
		return false;
	}
	DivisorStack[++DivisorStackPos]= newval;
	FixedPtMultiplier= 1.0 / newval;
	return true;
}

COMMAND(cglPopDivisor, "") {
	if (DivisorStackPos < 0)  {
		parsed->errmsg= "divisor stack underflow";
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
	char q, *out, *in, *rewrite;
	if (!input) return NULL;
	in= *input;
	/* skip delim characters */
	while (*in == ' ' || *in == '\t') in++;
	/* quoting support */
	if (*in == '\'' || *in == '"') {
		q= *in++;
		out= rewrite= in;
		while (*in && *in != q) {
			if (*in == '\\') {
				switch (* ++in) {
				case 'n': *in= '\n'; break;
				case 'r': *in= '\r'; break;
				case '\0': --in; break; /* don't run over end of string */
				default: break; /* every other value remains itself */
				}
			}
			*rewrite++ = *in++;
		}
		if (*in) in++; /* advance past the end-quote, if not end of string */
		*rewrite= '\0';
		/* always consider a token found, even if empty or missing end quote */
	}
	/* else just look for end of token */
	else {
		out= in;
		while (*in && *in != ' ' && *in != '\t') in++;
		if (*in) *in++= '\0';
		if (!*out) out= NULL; /* return null unless found a token */
	}
	while (*in == ' ' || *in == '\t') in++;
	*input= in; /* give caller back the modified pointer */
	return out;
}

char *sanitize_for_print(char *string) {
	char *c;
	for (c= string; *c; c++)
		if (*c <= ' ' || *c == 0x7F)
			*c= '_';
	return string;
}

/* ProcessCommand bites off the first word and dispatches to a command of that name.
 * It destroys "Line" with strtok in the process.
 */
int ProcessCommand(char *Line) {
	const CmdListEntry *Cmd;
	struct ParseParamsResult parsed;
	int GLErr;
	
	parsed.iCnt= parsed.lCnt= parsed.fCnt= parsed.dCnt= parsed.sCnt= parsed.oCnt= 0;
	parsed.errmsg= NULL;

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
	if (!ParseParams(&Line, Cmd->ArgFormat, &parsed)) {
		fprintf(stderr, "Error parsing params of %s%s%s\n", command,
			parsed.errmsg? ": ":"\n", parsed.errmsg? parsed.errmsg : "");
		return P_CMD_ERR;
	}
	/* dispatch command. Command may return extra error info in errbuf */
	if (!Cmd->Handler(&parsed)) {
		fprintf(stderr, "Error during execution of %s%s%s\n", command,
			parsed.errmsg? ": ":"\n", parsed.errmsg? parsed.errmsg : "");
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

bool ParseParams(char **line, const char *format, struct ParseParamsResult *parsed) {
	const char *tok= "", *fmt_next;
	SymbVarEntry *obj;
	const char *autocreate_name[sizeof(parsed->objects)/sizeof(parsed->objects[0])];
	int autocreate_type[sizeof(parsed->objects)/sizeof(parsed->objects[0])];
	int objtyp, autocreate= 0, optional= 0, repeat= 0, oCnt_start= parsed->oCnt, i;
	
	while (*format) {
		//DEBUGMSG(("Parse %s from \"%s\"\n", format, *line));
		/* parse out the next format specifier */
		fmt_next= format+1;
		autocreate= (*fmt_next == '!');
		if (autocreate) fmt_next++;
		optional= (*fmt_next == '?');
		if (optional) fmt_next++;
		repeat= (*fmt_next == '*');
		if (repeat) {
			++fmt_next;
			/* repeat must be end of list, otherwise no way to know how many it captured */
			assert(*fmt_next == '\0');
		}
		
		/* if there is no input left to parse, consider 'optional' and return accordingly */
		if (!**line) {
			if (optional || repeat) break;
			else {
				parsed->errmsg= "Not enough arguments";
				return false;
			}
		}
		
		switch (*format) {
		// Integer value
		case 'i': if (!ParseInt(line, parsed)) return false; break;
		//case 'l': if (!ParseLong(line, parsed)) return false; break;
		// Floating value
		case 'f': if (!ParseFloat(line, parsed)) return false; break;
		case 'd': if (!ParseDouble(line, parsed)) return false; break;
		// Color, either "#xxxxxx" or 3xFloat or 4xFloat (always stored as 4x float)
		case 'c': if (!ParseColor(line, parsed)) return false; break;
		// Single Token string
		case 't': if (!ParseToken(line, parsed)) return false; break;
		// remainder of buffer
		case 'b': if (!CaptureRemainder(line, parsed)) return false; break;
		// string, either one token or rest of line if quoted
		case 's':
		// File name, same as string but check that it exists
		case '/':
			if (!(**line == '"'? ParseToken(line, parsed) : CaptureRemainder(line, parsed)))
				return false;
			assert(parsed->sCnt > 0);
			tok= parsed->strings[parsed->sCnt - 1];
			if (*format == '/' && !FileExists(tok)) {
				snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "No such file '%s'", tok);
				parsed->errmsg= parsed->errmsg_buf;
				return false;
			}
			break;
		// Display list
		if (0) case 'L': objtyp= NAMED_LIST;
		// Quadric
		if (0) case 'Q': objtyp= NAMED_QUADRIC;
		// Texture
		if (0) case 'T': objtyp= NAMED_TEXTURE;
		// Font
		if (0) case 'F': objtyp= NAMED_FONT;
			
			if (parsed->oCnt >= sizeof(parsed->objects)/sizeof(parsed->objects[0])) {
				parsed->errmsg= "Argument count exceeded (object)";
				return false;
			}
			tok= next_token(line);
			if (!tok || !*tok) {
				snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Expected named %s", SymbVarTypeName[objtyp]);
				parsed->errmsg= parsed->errmsg_buf;
				return false;
			}
			else if ((obj= GetSymbVar(tok, objtyp))) {
				parsed->objects[parsed->oCnt++]= obj;
			}
			else if (autocreate) {
				/* Doesn't exist.  If autocreate, wait until rest of parsing has succeeded.
				   Otherwise the handler won't get to see it, and could end up with dangling
				   NULL pointers in the variables tree for things like fonts. */
				autocreate_name[parsed->oCnt]= tok;
				autocreate_type[parsed->oCnt]= objtyp;
				parsed->objects[parsed->oCnt++]= NULL;
			}
			else {
				snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Named %s '%s' does not exist",
					SymbVarTypeName[objtyp], tok);
				parsed->errmsg= parsed->errmsg_buf;
				return false;
			}
			break;
		default:
			assert(0 == "all formats handled");
			return false;
		}
		/* advance to next format specifier */
		if (!repeat) format= fmt_next;
	}
	/* all format specifiers have been satisfied.  Check for leftover input */
	if (**line) {
		parsed->errmsg= "unpexpected extra arguments";
		return false;
	}
	/* Now, for any object that didn't exist and was marked autocreate, create it. */
	for (i= oCnt_start; i < parsed->oCnt; i++) {
		if (!parsed->objects[i]) {
			assert(autocreate_type[i] >= 1 && autocreate_type[i] <= 4);
			assert(autocreate_name[i] != NULL && *autocreate_name[i]);
			parsed->objects[i]= CreateSymbVar(autocreate_name[i], autocreate_type[i]);
			switch (autocreate_type[i]) {
			case NAMED_LIST:    parsed->objects[i]->Value= glGenLists(1); break;
			case NAMED_QUADRIC: parsed->objects[i]->Data= gluNewQuadric(); break;
			case NAMED_TEXTURE: glGenTextures(1, (GLuint*) &(parsed->objects[i]->Value)); break;
			/* can't auto-create FTfont* object.  Caller needs to handle that. */
			default:            parsed->objects[i]->Data= NULL; break;
			}
		}
	}
	return true;
}

bool ParseInt(char **line, struct ParseParamsResult *parsed) {
	const IntConstListEntry *SymConst;
	char *tok, *EndPtr;
	long l;

	if (parsed->iCnt >= sizeof(parsed->ints)/sizeof(parsed->ints[0])) {
		parsed->errmsg= "Argument count exceeded (int)";
		return false;
	}
	if (!(tok= next_token(line)) || !*tok) {
		parsed->errmsg= "expected Integer";
		return false;
	}
	
	if (tok[0] >= '0' && tok[0] <= '9') {
		l= strtol(tok, &EndPtr, (tok[0] == '0' && tok[1] == 'x')? 16 : 10);
		if (*EndPtr == '\0') {
			parsed->ints[parsed->iCnt++]= l;
			return true;
		} else {
			snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Can't parse int '%s'", tok);
			parsed->errmsg= parsed->errmsg_buf;
			return false;
		}
	}
	else {
		//DEBUGMSG(("Searching for %s...", tok));
		if ((SymConst= GetIntConst(tok))) {
			//DEBUGMSG((" Found: %i\n", SymConst->Value));
			parsed->ints[parsed->iCnt++]= SymConst->Value;
			return true;
		}
		else {
			//DEBUGMSG(("not found.\n"));
			snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unknown constant '%s'", tok);
			parsed->errmsg= parsed->errmsg_buf;
			return false;
		}
	}
}

bool ParseFloat(char **line, struct ParseParamsResult *parsed) {
	if (parsed->fCnt >= sizeof(parsed->floats)/sizeof(parsed->floats[0])) {
		parsed->errmsg= "Argument count exceeded (float)";
		return false;
	}
	if (!ParseDouble(line, parsed)) return false;
	parsed->floats[parsed->fCnt++]= parsed->doubles[--parsed->dCnt];
	return true;
}

bool ParseDouble(char **line, struct ParseParamsResult *parsed) {
	char *tok, *EndPtr;
	double num, denom;
	
	if (parsed->dCnt >= sizeof(parsed->doubles)/sizeof(parsed->doubles[0])) {
		parsed->errmsg= "Argument count exceeded (double)";
		return false;
	}
	if (!(tok= next_token(line))) {
		parsed->errmsg= "expected Float";
		return false;
	}
	/* ignore double negatives */
	if (tok[0] == '-' && tok[1] == '-') tok+= 2;
	num= strtod(tok, &EndPtr);
	if (*EndPtr == '/') {
		denom= strtod(EndPtr+1, &EndPtr);
		if (!denom) {
			parsed->errmsg= "Division by zero";
			return false;
		}
		num /= denom;;
	} else {
		num *= FixedPtMultiplier;
	}
	if (*EndPtr != '\0') {
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Can't parse float '%s'", tok);
		parsed->errmsg= parsed->errmsg_buf;
		return false;
	}
	parsed->doubles[parsed->dCnt++]= num;
	return true;
}

bool ParseToken(char **line, struct ParseParamsResult *parsed) {
	char *tok;
	/* buffer space check */
	if (parsed->sCnt >= sizeof(parsed->strings)/sizeof(parsed->strings[0])) {
		parsed->errmsg= "Argument count exceeded (string)";
		return false;
	}
	if (!(tok= next_token(line))) {
		parsed->errmsg= "expected Token";
		return false;
	}
	parsed->strings[parsed->sCnt++]= tok;
	return true;
}

bool CaptureRemainder(char **line, struct ParseParamsResult *parsed) {
	if (parsed->sCnt >= sizeof(parsed->strings)/sizeof(parsed->strings[0])) {
		parsed->errmsg= "Argument count exceeded (string)";
		return false;
	}
	parsed->strings[parsed->sCnt++]= *line;
	*line += strlen(*line);
	return true;
}

bool ParseHexByte(const char *str, float *Result);
bool ParseHexColor(const char* str, float Result[4]);
bool ParseColor(char **line, struct ParseParamsResult *parsed) {
	int n;
	
	/* always stored as four floating point components */
	if (parsed->fCnt + 4 > sizeof(parsed->floats)/sizeof(parsed->floats[0])) {
		parsed->errmsg= "Argument count exceeded (float)";
		return false;
	}
	// hash indicates a hex code
	if (**line == '#' || ((**line == '"' || **line == '\'') && (*line)[1] == '#')) {
		if (!ParseHexColor(next_token(line), parsed->floats + parsed->fCnt)) {
			parsed->errmsg= "Invalid color code";
			return false;
		} else {
			parsed->fCnt+= 4;
			return true;
		}
	}
	else {
		// else look for 3 or 4 numbers
		for (n= 0; n < 4; n++) {
			if (!ParseFloat(line, parsed))
				break;
		}
		if (n < 3) {
			parsed->fCnt -= n;
			parsed->errmsg= "Not enough components for color";
			return false;
		}
		/* alpha= 1.0 unless specified otherwise */
		if (n < 4) parsed->floats[parsed->fCnt++]= 1.0;
		return true;
	}
}
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
	return true;
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

bool FileExists(const char *Name) {
#ifdef _WIN32
	return PathFileExists(Name);
#else
	struct stat s;
	return stat(Name, &s) == 0;
#endif
}
