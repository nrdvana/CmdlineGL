#include "Global.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "GlHeaders.h"
#include "ParseGL.h"
#include "SymbolHash.h"
#include "ImageLoader.h"
#include "Font.h"

/* For anyone who wants to reuse code from this file, or even just make it
 *   thread-safe, (heaven help you ;-) you will need to make a non-static
 *   instance of ScanParamsResult, and edit ScanParams so it takes that as a
 *   parameter.  you will also need to edit all the functions so that they
 *   use the return value of ScanParams.
 */
ScanParamsResult ParseResult;

/* CmdlineGL supports fixed-point numbers on stdin, by multiplying
 * all float values by this multiplier.  Naturally, it defaults to 1.0
 */
double FixedPtMultiplier= 1.0;

bool ParseInt(const char* Text, GLint *Result);
bool ParseFloat(const char* Text, GLfloat *Result);
bool ParseDouble(const char* Text, GLdouble *Result);
bool ParseColor(const char* Text, GLubyte *Result);
bool ParseSymbVar(const char* Text, const SymbVarEntry **Result, int Type);

const SymbVarEntry* CreateNamedObj(const char* Name, int Type);
int ReportMissingObj(const char *Name);

//----------------------------------------------------------------------------
// CmdlineGL Functions
//
PUBLISHED(cglFixedPt, DoSetFixedPoint) {
	char *EndPtr;
	double newval;
	if (argc != 1) return ERR_PARAMCOUNT;
	// We don't want to scale the new scale... so don't use ScanParams("d").
	// That was a fun bug, lol.
	newval= strtod(argv[0], &EndPtr);
	if (*EndPtr != '\0') return ERR_PARAMPARSE;

	FixedPtMultiplier= 1.0 / newval;
	return 0;
}

//----------------------------------------------------------------------------
PUBLISHED(cglSwapBuffers,DoSwapBuffers) {
	if (argc != 0) return ERR_PARAMCOUNT;
	SDL_GL_SwapBuffers();
	return 0;
}

//----------------------------------------------------------------------------
// Setup Functions
//
PUBLISHED(glMatrixMode, DoMatrixMode) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv, &ParseResult)) return ERR_PARAMPARSE;
	glMatrixMode(ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(glEnable, DoEnable) {
	int i;
	if (argc < 1 || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (!ScanParams("i*", argv, &ParseResult)) return ERR_PARAMPARSE;
	for (i=0; i<argc; i++)
		glEnable(ParseResult.Ints[i]);
	return 0;
}
PUBLISHED(glDisable, DoDisable) {
	int i;
	if (argc < 1 || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (!ScanParams("i*", argv, &ParseResult)) return ERR_PARAMPARSE;
	for (i=0; i<argc; i++)
		glDisable(ParseResult.Ints[i]);
	return 0;
}
PUBLISHED(glHint, DoHint) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv, &ParseResult)) return ERR_PARAMPARSE;
	glHint(ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}
PUBLISHED(glClear, DoClear) {
	int flags;
	if (argc < 1 || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (!ScanParams("i*", argv, &ParseResult)) return ERR_PARAMPARSE;
	flags= 0;
	while (argc--)
		flags|= ParseResult.Ints[argc];
	glClear(flags);
	return 0;
}
PUBLISHED(glClearColor, DoClearColor) {
	if (argc != 1 && argc != 3 && argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("c", argv, &ParseResult)) return ERR_PARAMPARSE;
	glClearColor(ParseResult.Floats[0], ParseResult.Floats[1], ParseResult.Floats[2], ParseResult.Floats[3]);
	return 0;
}
PUBLISHED(glClearDepth, DoClearDepth) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("f", argv, &ParseResult)) return ERR_PARAMPARSE;
	glClearDepth(ParseResult.Floats[0]);
	return 0;
}
PUBLISHED(glBegin, DoBegin) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (IsGlBegun)
		fprintf(stderr, "Error: multiple calls to glBegin without glEnd\n");
	glBegin(ParseResult.Ints[0]);
	IsGlBegun= true;
	return 0;
}
PUBLISHED(glEnd, DoEnd) {
	if (argc != 0) return ERR_PARAMCOUNT;
	if (!IsGlBegun)
		fprintf(stderr, "Error: glEnd without glBegin\n");
	glEnd();
	IsGlBegun= false;
	return 0;
}
PUBLISHED(glFlush, DoFlush) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glFlush();
	return 0;
}

//----------------------------------------------------------------------------
// Vertex Functions
//
PUBLISHED(glVertex, DoVertex) {
	if (!ScanParams("d*", argv, &ParseResult)) return ERR_PARAMPARSE;
	switch (argc) {
	case 2: glVertex2dv(ParseResult.Doubles); break;
	case 3: glVertex3dv(ParseResult.Doubles); break;
	case 4: glVertex4dv(ParseResult.Doubles); break;
	default:
		return ERR_PARAMCOUNT;
	}
	return 0;
}
PUBLISHED(glNormal, DoNormal) {
	if (argc != 3) return ERR_PARAMCOUNT;
	if (!ScanParams("ddd", argv, &ParseResult)) return ERR_PARAMPARSE;
	glNormal3dv(ParseResult.Doubles);
	return 0;
}

//----------------------------------------------------------------------------
// Color Functions
//
PUBLISHED(glColor, DoColor) {
	if (argc != 1 && argc != 3 && argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("c", argv, &ParseResult)) return ERR_PARAMPARSE;
	glColor4fv(ParseResult.Floats);
	return 0;
}
PUBLISHED(glFog, DoFog) {
	int mode, iTemp;
	float fTemp;
	if (argc < 2) return ERR_PARAMCOUNT;
	// The parameter to this one really matters, since floats get fixed-point
	//  multiplied, and colors need special treatment.
	if (!ParseInt(argv[0], &mode)) return ERR_PARAMPARSE;
	switch (mode) {
	case GL_FOG_MODE:
	case GL_FOG_INDEX:
		if (!ParseInt(argv[1], &iTemp)) return ERR_PARAMPARSE;
		glFogi(mode, iTemp);
		break;
	case GL_FOG_DENSITY:
	case GL_FOG_START:
	case GL_FOG_END:
		if (!ParseFloat(argv[1], &fTemp)) return ERR_PARAMPARSE;
		glFogf(mode, fTemp);
		break;
	case GL_FOG_COLOR:
		if (argc != 2 && argc != 4 && argc != 5) return ERR_PARAMCOUNT;
		if (!ScanParams("c", argv+1, &ParseResult)) return ERR_PARAMPARSE;
		glFogfv(mode, ParseResult.Floats);
		break;
	}
	return 0;
}
PUBLISHED(glLight, DoLight) {
	int mode, light;
	if (argc < 3) return ERR_PARAMCOUNT;
	if (!ParseInt(argv[0], &light)) return ERR_PARAMPARSE;
	if (!ParseInt(argv[1], &mode)) return ERR_PARAMPARSE;
	argc-=2;
	switch (mode) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
		if (argc != 1 && argc != 3 && argc != 4) return ERR_PARAMCOUNT;
		if (!ScanParams("c", argv+2, &ParseResult)) return ERR_PARAMPARSE;
		glLightfv(light, mode, ParseResult.Floats);
		break;
	case GL_POSITION:
		// take care of the situation where the user only passes 3 params for the point
		// if they screw it up any worse than that, its their own fault.
		ParseResult.Floats[3]= 0.0f;
	default:
		if (!ScanParams("f*", argv+2, &ParseResult)) return ERR_PARAMPARSE;
		glLightfv(light, mode, ParseResult.Floats);
	}
	return 0;
}
PUBLISHED(glLightModel, DoLightModel) {
	if (argc < 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (ParseResult.Ints[0] == GL_LIGHT_MODEL_AMBIENT) {
		if (!ScanParams("c", argv+1, &ParseResult)) return ERR_PARAMPARSE;
	}
	else
		if (!ScanParams("f*", argv+1, &ParseResult)) return ERR_PARAMPARSE;
	glLightModelfv(ParseResult.Ints[0], ParseResult.Floats);
	return 0;
}
PUBLISHED(glShadeModel, DoShadeModel) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv, &ParseResult)) return ERR_PARAMPARSE;
	glShadeModel(ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(glMaterial, DoMaterial) {
	int face;
	if (argc <= 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (ParseResult.Ints[1] == GL_COLOR_INDEXES) {
		// Handle color indicies as 3 mandatory integers
		face= ParseResult.Ints[0];
		if (argc != 5) return ERR_PARAMCOUNT;
		if (!ScanParams("iii", argv+2, &ParseResult)) return ERR_PARAMPARSE;
		glMaterialiv(face, GL_COLOR_INDEXES, ParseResult.Ints);
	}
	else {
		// Handle everything else as a color.
		if (argc != 3 && argc != 5 && argc != 6) return ERR_PARAMCOUNT;
		if (!ScanParams("c", argv+2, &ParseResult)) return ERR_PARAMPARSE;
		glMaterialfv(ParseResult.Ints[0], ParseResult.Ints[1], ParseResult.Floats);
	}
	return 0;
}
PUBLISHED(glColorMaterial, DoColorMaterial) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv, &ParseResult)) return ERR_PARAMPARSE;
	glLightModelf(ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}
PUBLISHED(glBlendFunc, DoBlendFunc) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv, &ParseResult)) return ERR_PARAMPARSE;
	glBlendFunc(ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}

//----------------------------------------------------------------------------
// Texture Functions
//
PUBLISHED(glBindTexture, DoBindTexture) {
	const SymbVarEntry* NamedObj;
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("iT", argv, &ParseResult)) return ERR_PARAMPARSE;
	NamedObj= ParseResult.Symbolics[0];
	if (!NamedObj) NamedObj= CreateNamedObj(argv[1], NAMED_TEXTURE);
	glBindTexture(ParseResult.Ints[0], NamedObj->Value);
	return 0;
}
PUBLISHED(glTexParameter, DoTexParameter) {
	const int FIXED_PARAMS= 2;
	if (argc <= FIXED_PARAMS || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (argc == 3)
		if (ScanParams("iii", argv, &ParseResult)) {
			glTexParameteri(ParseResult.Ints[0], ParseResult.Ints[1], ParseResult.Ints[2]);
			return 0;
		}
	// 3-integer conversion failed, so maybe it's an array of floats
	if (ScanParams("ii", argv, &ParseResult) && ScanParams("f*", argv+FIXED_PARAMS, &ParseResult))
		glTexParameterfv(ParseResult.Ints[0], ParseResult.Ints[1], ParseResult.Floats);
	else
		return ERR_PARAMPARSE;
	return 0;
}
PUBLISHED(glTexCoord, DoTexCoord) {
	if (argc >= 1 && argc <= 4) {
		if (!ScanParams("d*", argv, &ParseResult)) return ERR_PARAMPARSE;
		switch (argc) {
		case 1: glTexCoord1dv(ParseResult.Doubles); break;
		case 2: glTexCoord2dv(ParseResult.Doubles); break;
		case 3: glTexCoord3dv(ParseResult.Doubles); break;
		case 4: glTexCoord4dv(ParseResult.Doubles); break;
		}
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(cglNewFont, DoNewFont) {
	const SymbVarEntry* NamedObj;
	SDL_Surface *Img;
	bool Success;
	if (argc < 3) return ERR_PARAMCOUNT;
	if (!ScanParams("iFN", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (ParseResult.Ints[0] != CGL_BMPFONT) return ERR_PARAMPARSE;
	NamedObj= ParseResult.Symbolics[0];
	if (!NamedObj) NamedObj= CreateNamedObj(argv[1], NAMED_FONT);
	Img= LoadImg(ParseResult.FName);
	if (!Img) return ERR_EXEC;
	Success= GenerateFont(Img, (Font*) NamedObj->Data);
	SDL_FreeSurface(Img);
	return Success? 0 : ERR_EXEC;
}

//----------------------------------------------------------------------------
// Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glLoadIdentity();
	return 0;
}
PUBLISHED(glLoadMatrix, DoLoadMatrix) {
	if (argc != 16) return ERR_PARAMCOUNT;
	if (!ScanParams("d*", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (ParseResult.ParamsParsed != 16) return ERR_PARAMPARSE;
	glLoadMatrixd(ParseResult.Doubles);
	return 0;
}
PUBLISHED(glPushMatrix, DoPushMatrix) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glPushMatrix();
	return 0;
}
PUBLISHED(glPopMatrix, DoPopMatrix) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glPopMatrix();
	return 0;
}
PUBLISHED(glMultMatrix, DoMultMatrix) {
	if (argc != 16) return ERR_PARAMCOUNT;
	if (!ScanParams("d*", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (ParseResult.ParamsParsed != 16) return ERR_PARAMPARSE;
	glMultMatrixd(ParseResult.Doubles);
	return 0;
}
PUBLISHED(glScale, DoScale) {
	if (argc == 3) {
		if (!ScanParams("ddd", argv, &ParseResult)) return ERR_PARAMPARSE;
		glScaled(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2]);
	}
	else if (argc == 2) {
		if (!ScanParams("dd", argv, &ParseResult)) return ERR_PARAMPARSE;
		glScaled(ParseResult.Doubles[0], ParseResult.Doubles[1], 1);
	}
	else if (argc == 1) {
		if (!ScanParams("d", argv, &ParseResult)) return ERR_PARAMPARSE;
		glScaled(ParseResult.Doubles[0], ParseResult.Doubles[0], ParseResult.Doubles[0]);
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glTranslate, DoTranslate) {
	if (argc == 3) {
		if (!ScanParams("ddd", argv, &ParseResult)) return ERR_PARAMPARSE;
		glTranslated(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2]);
	}
	else if (argc == 2) {
		if (!ScanParams("dd", argv, &ParseResult)) return ERR_PARAMPARSE;
		glTranslated(ParseResult.Doubles[0], ParseResult.Doubles[1], 0);
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glRotate, DoRotate) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("dddd", argv, &ParseResult)) return ERR_PARAMPARSE;
	glRotated(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2], ParseResult.Doubles[3]);
	return 0;
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
PUBLISHED(glViewport, DoViewport) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("iiii", argv, &ParseResult)) return ERR_PARAMPARSE;
	glViewport(ParseResult.Ints[0], ParseResult.Ints[1], ParseResult.Ints[2], ParseResult.Ints[3]);
	return 0;
}
PUBLISHED(glOrtho, DoOrtho) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("dddddd", argv, &ParseResult)) return ERR_PARAMPARSE;
	glOrtho(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2], ParseResult.Doubles[3], ParseResult.Doubles[4], ParseResult.Doubles[5]);
	return 0;
}
PUBLISHED(glFrustum, DoFrustum) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("dddddd", argv, &ParseResult)) return ERR_PARAMPARSE;
	glFrustum(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2], ParseResult.Doubles[3], ParseResult.Doubles[4], ParseResult.Doubles[5]);
	return 0;
}

//----------------------------------------------------------------------------
// Display List Functions
//
PUBLISHED(glNewList, DoNewList) {
	const SymbVarEntry* NamedObj;
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Li", argv, &ParseResult)) return ERR_PARAMPARSE;
	NamedObj= ParseResult.Symbolics[0];
	if (!NamedObj) NamedObj= CreateNamedObj(argv[0], NAMED_LIST);
	glNewList(NamedObj->Value, ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(glEndList, DoEndList) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glEndList();
	return 0;
}
PUBLISHED(glCallList, DoCallList) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("L", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	glCallList(ParseResult.Symbolics[0]->Value);
	return 0;
}

//----------------------------------------------------------------------------
// Glu Functions
//
PUBLISHED(gluLookAt, DoLookAt) {
	if (argc != 9) return ERR_PARAMCOUNT;
	if (!ScanParams("ddddddddd", argv, &ParseResult)) return ERR_PARAMPARSE;
	gluLookAt(ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2],
		ParseResult.Doubles[3], ParseResult.Doubles[4], ParseResult.Doubles[5],
		ParseResult.Doubles[6], ParseResult.Doubles[7], ParseResult.Doubles[8]);
	return 0;
}
PUBLISHED(gluNewQuadric, DoNewQuadric) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("Q", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0])
		CreateNamedObj(argv[0], NAMED_QUADRIC);
	return 0;
}
PUBLISHED(gluQuadricDrawStyle, DoQuadricDrawStyle) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Qi", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluQuadricDrawStyle((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(gluQuadricNormals, DoQuadricNormals) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Qi", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluQuadricNormals((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(gluQuadricOrientation, DoQuadricOrientation) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Qi", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluQuadricOrientation((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(gluQuadricTexture, DoQuadricTexture) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Qi", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluQuadricTexture((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Ints[0]);
	return 0;
}
PUBLISHED(gluCylinder, DoCylinder) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("Qdddii", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluCylinder((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Doubles[2], ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}
PUBLISHED(gluSphere, DoSphere) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("Qdii", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluSphere((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Doubles[0], ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}
PUBLISHED(gluDisk, DoDisk) {
	if (argc != 5) return ERR_PARAMCOUNT;
	if (!ScanParams("Qddii", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluDisk((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Ints[0], ParseResult.Ints[1]);
	return 0;
}
PUBLISHED(gluPartialDisk, DoPartialDisk) {
	if (argc != 7) return ERR_PARAMCOUNT;
	if (!ScanParams("Qddiidd", argv, &ParseResult)) return ERR_PARAMPARSE;
	if (!ParseResult.Symbolics[0]) return ReportMissingObj(argv[0]);
	gluPartialDisk((GLUquadric*)ParseResult.Symbolics[0]->Data, ParseResult.Doubles[0], ParseResult.Doubles[1], ParseResult.Ints[0], ParseResult.Ints[1], ParseResult.Doubles[2], ParseResult.Doubles[3]);
	return 0;
}

//----------------------------------------------------------------------------
// Parsing Functions
//
bool ScanParams(const char* ParamType, char** Args, ScanParamsResult* Result) {
	GLubyte colorVals[4];
	GLint *iParam= Result->Ints;
	GLfloat *fParam= Result->Floats;
	GLdouble *dParam= Result->Doubles;
	const SymbVarEntry **sParam= Result->Symbolics;
	int i;
	char *ch;
	Result->ParamsParsed= 0;
	bool Success= true;
	while (Success && *Args && *ParamType != '\0') {
		if (*ParamType == '*') // replay last value, infinitely
			ParamType--;
		switch (*ParamType) {
		// Integer value
		case 'i': Success= ParseInt(*Args, iParam++); break;
		// Float value
		case 'f': Success= ParseFloat(*Args, fParam++); break;
		// Double value
		case 'd': Success= ParseDouble(*Args, dParam++); break;
		// Display list
		case 'L': Success= ParseSymbVar(*Args, sParam++, NAMED_LIST); break;
		// Quadric
		case 'Q': Success= ParseSymbVar(*Args, sParam++, NAMED_QUADRIC); break;
		// Texture
		case 'T': Success= ParseSymbVar(*Args, sParam++, NAMED_TEXTURE); break;
		// Font
		case 'F': Success= ParseSymbVar(*Args, sParam++, NAMED_FONT); break;
		// Color, either "#xxxxxx" or 3xFloat or 4xFloat
		case 'c':
			if (**Args == '#' && ParseColor(&Args[0][1], colorVals)) {
				for (i=0; i<4; i++)
					*fParam++ = ((float)colorVals[i])*(1.0/255);
			}
			else {
				Success= ParseFloat(Args[0], fParam++);
				for (i=1; i<4 && Success && Args[i]; i++)
					Success= ParseFloat(Args[i], fParam++);
				if (i < 3)
					Success= false;
				else {
					if (i==3) *fParam++ = 1.0;
					Args+= (i-1);
				}
			}
			break;
		// File Name
		case 'N':
			if (ParamType[1] != '\0') {
				fprintf(stderr, "ScanParams: File names can only be the last argument to read, due to possible embedded spaces.\n");
				Success= false;
			}
			// XXX WARNING!! DANGEROUS AND SINISTER (but fun) KLUGE FOLLOWS! XXX
			// We know that all the arg strings came from a single string which is
			//   still in our global read-buffer, so to reconstruct the filename
			//   with spaces in it, we can simply replace a few NULs with spaces.
			// (I should enter the IOCCC some day...)
			else
				for (Result->FName= *Args++; *Args; Args++)
					for (ch=*(Args-1); !*ch; *ch--=' ');
			// Note: if someone feels like fixing this someday, the best thing would be
			// not to have broken up the original string.  In other words, let each one
			// of these 'Do' functions parse its own stuff out of one single string.  It
			// would even be more efficient since the string would then only get scanned
			// once.  This, however, would require a lot of rewriting...  wish I'd
			// thought of it sooner.
			break;
		// Catch programming errors
		default:
			fprintf(stderr, "ScanParams: Unknown param type '%c'!  (bug)\n", *ParamType);
			Success= false;
		}
		Args++;
		Result->ParamsParsed++;
		ParamType++;
	}
	return Success;
}

bool ParseInt(const char* Text, GLint *Result) {
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

bool ParseSymbol(const char* Text, int *Result) {
	const SymbVarEntry *SymbVar;
	SymbVar= GetSymbVar(Text);
	if (SymbVar) {
		*Result= SymbVar->Value;
		return true;
	}
	else return false;
}

bool ParseFloat(const char* Text, GLfloat *Result) {
	double val;
	bool status= ParseDouble(Text, &val);
	if (status) *Result= val;
	return status;
}

bool ParseDouble(const char* Text, GLdouble *Result) {
	char *EndPtr;
	if (Text[0] == '-' && Text[1] == '-') Text+= 2; // be nice about double negatives
	*Result= FixedPtMultiplier * strtod(Text, &EndPtr);
	return (*EndPtr == '\0');
}

bool ParseColor(const char* Text, GLubyte *Result) {
	int i, hexval;
	char *StopPos;
	hexval= strtol(Text, &StopPos, 16);
	// Check for alpha component
	switch (StopPos-Text) {
	case 8: // has an alpha.
		Result[3]= hexval & 0xFF;
		hexval>>= 8;
		break;
	case 6: // no alpha, so default to 1.0
		Result[3]= 0xFF;
		break;
	default: // invalid number of hex chars
		return false;
	}
	Result[2]= hexval & 0xFF;
	Result[1]= (hexval>>8) & 0xFF;
	Result[0]= (hexval>>16) & 0xFF;
	return true;
}

bool ParseSymbVar(const char* Text, const SymbVarEntry **Result, int Type) {
	const SymbVarEntry *Entry= GetSymbVar(Text);

	if (Entry && Entry->Type != Type) {
		fprintf(stdout, "Named object \"%s\" is not of type %s (it's a %s)\n", Text, SymbVarTypeName[Type], SymbVarTypeName[Entry->Type]);
		return false;
	}
	*Result= Entry;
	return true;
}

const SymbVarEntry* CreateNamedObj(const char* Name, int Type) {
	SymbVarEntry *NewEntry;
	NewEntry= CreateSymbVar(Name); // entry is added to RB tree at this point
	NewEntry->Type= Type;
	switch (Type) {
	case NAMED_LIST: NewEntry->Value= glGenLists(1); break;
	case NAMED_QUADRIC: NewEntry->Data= gluNewQuadric(); break;
	case NAMED_TEXTURE: glGenTextures(1, &(NewEntry->Value)); break;
	case NAMED_FONT: NewEntry->Data= Font_Initialize(malloc(sizeof(Font))); break;
	}
	DEBUGMSG(("Created named object %s = %d\n", Name, NewEntry->Value));
	return NewEntry;
}

int ReportMissingObj(const char *Name) {
	fprintf(stdout, "Named object: \"%s\" does not exist.\n", Name);
	return ERR_PARAMPARSE;
}
