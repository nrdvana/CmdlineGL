#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "GlHeaders.h"
#include "ParseGL.h"
#include "SymbolHash.h"
#include "ImageLoader.h"
	
const char* VAR_INT="iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
#define VAR_INT_LEN 32
const char* VAR_FLOAT="ffffffffffffffffffffffffffffffff";
#define VAR_FLOAT_LEN 32
const char* VAR_DBL="dddddddddddddddddddddddddddddddd";
#define VAR_DBL_LEN 32

#if VAR_INT_LEN != MAX_GL_PARAMS
  #error Edit the strings in ParseGL.h to match the value of MAX_GL_PARAMS
#endif


GLint iParams[MAX_GL_PARAMS];
GLfloat fParams[MAX_GL_PARAMS];
GLdouble dParams[MAX_GL_PARAMS];
const SymbVarEntry* sParams[MAX_GL_PARAMS];

double FixedPtMultiplier= 1.0;

bool ScanParams(const char* ParamType, char** Args);
bool ParseInt(const char* Text, GLint *Result);
bool ParseFloat(const char* Text, GLfloat *Result);
bool ParseDouble(const char* Text, GLdouble *Result);
bool ParseColor(const char* Text, GLubyte *Result);
bool ParseSymbVar(const char* Text, const SymbVarEntry **Result, bool AutoCreate, int Type);

//----------------------------------------------------------------------------
// CmdlineGL Functions
//
PUBLISHED(cglFixedPt, DoSetFixedPoint) {
	char *EndPtr;
	double newval;
	if (argc != 1) return ERR_PARAMCOUNT;
	// LOL, we don't want to scale the new scale... so don't use ScanParams("d")
	newval= strtod(argv[0], &EndPtr);
	if (*EndPtr != '\0') return ERR_PARAMPARSE;

	FixedPtMultiplier= 1.0 / newval;
	return 0;
}
PUBLISHED(cglLoadImage2D, DoLoadImage2D) {
	int i, j;
	Image Img;
	if (argc < 1) return ERR_PARAMCOUNT;
	// if more than 2 params, assume its a filename with spaces in it.
	if (argc > 1) {
		// XXX WARNING!! DANGEROUS AND SINISTER (but fun) KLUGE FOLLOWS! XXX
		// We know that all the arg strings came from a single string which is
		//   still in our global read-buffer, so to reconstruct the filename
		//   with spaces in it, we can simply replace a few NULs with spaces.
		// (I should enter the IOCCC someday...)
		for (i=1; i<argc; i++)
			for (j=-1; !argv[i][j]; j--)
				argv[i][j]= ' ';
		// Note: if someone feels like fixing this someday, the best thing would be
		// not to have broken up the original string.  In other words, let each one
		// of these 'Do' functions parse its own stuff out of one single string.  It
		// would even be more efficient since the string would then only get scanned
		// once.  This, however, would require a lot of rewriting...  wish I'd
		// thought of it sooner.
	}
	// Now load the image
	LoadImage(argv[0], &Img);
	// Then, load the image data into OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, 3, Img.Width, Img.Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Img.Data);
	free(Img.Data);
}

//----------------------------------------------------------------------------
// Setup Functions
//
PUBLISHED(glMatrixMode, DoMatrixMode) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glMatrixMode(iParams[0]);
	return 0;
}
PUBLISHED(glEnable, DoEnable) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glEnable(iParams[0]);
	return 0;
}
PUBLISHED(glDisable, DoDisable) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glDisable(iParams[0]);
	return 0;
}
PUBLISHED(glHint, DoHint) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv)) return ERR_PARAMPARSE;
	glHint(iParams[0], iParams[1]);
	return 0;
}
PUBLISHED(glClear, DoClear) {
	int flags;
	if (argc < 1 || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (!ScanParams(VAR_INT+(VAR_INT_LEN-argc), argv)) return ERR_PARAMPARSE;
	flags= 0;
	while (argc--)
		flags|= iParams[argc];
	glClear(flags);
	return 0;
}
PUBLISHED(glClearColor, DoClearColor) {
	if (argc != 1 && argc != 3 && argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("c", argv)) return ERR_PARAMPARSE;
	glClearColor(fParams[0], fParams[1], fParams[2], fParams[3]);
	return 0;
}
PUBLISHED(glClearDepth, DoClearDepth) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("f", argv)) return ERR_PARAMPARSE;
	glClearDepth(fParams[0]);
	return 0;
}
PUBLISHED(glBegin, DoBegin) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	if (IsGlBegun)
		fprintf(stderr, "Error: multiple calls to glBegin without glEnd\n");
	glBegin(iParams[0]);
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
	if (argc == 2) {
		if (!ScanParams("dd", argv)) return ERR_PARAMPARSE;
		glVertex2dv(dParams);
	}
	else if (argc == 3) {
		if (!ScanParams("ddd", argv)) return ERR_PARAMPARSE;
		glVertex3dv(dParams);
	}
	else if (argc == 4) {
		if (!ScanParams("dddd", argv)) return ERR_PARAMPARSE;
		glVertex4dv(dParams);
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glNormal, DoNormal) {
	if (argc != 3) return ERR_PARAMCOUNT;
	if (!ScanParams("ddd", argv)) return ERR_PARAMPARSE;
	glNormal3dv(dParams);
	return 0;
}

//----------------------------------------------------------------------------
// Color Functions
//
PUBLISHED(glColor, DoColor) {
	if (argc != 1 && argc != 3 && argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("c", argv)) return ERR_PARAMPARSE;
	glColor4fv(fParams);
	return 0;
}
PUBLISHED(glFog, DoFog) {
	int mode;
	if (argc < 2) return ERR_PARAMCOUNT;
	// The parameter to this one really matters, since floats get fixed-point
	//  multiplied, and colors need special treatment.
	if (!ParseInt(argv[0], &mode)) return ERR_PARAMPARSE;
	switch (mode) {
	case GL_FOG_MODE:
	case GL_FOG_INDEX:
		if (!ParseInt(argv[1], &iParams[0])) return ERR_PARAMPARSE;
		glFogi(mode, iParams[0]);
		break;
	case GL_FOG_DENSITY:
	case GL_FOG_START:
	case GL_FOG_END:
		if (!ParseFloat(argv[1], &fParams[0])) return ERR_PARAMPARSE;
		glFogf(mode, fParams[0]);
		break;
	case GL_FOG_COLOR:
		if (argc != 2 && argc != 4 && argc != 5) return ERR_PARAMCOUNT;
		if (!ScanParams("c", argv+1)) return ERR_PARAMPARSE;
		glFogfv(mode, fParams);
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
		if (!ScanParams("c", argv+2)) return ERR_PARAMPARSE;
		glLightfv(light, mode, fParams);
		break;
	case GL_POSITION:
		// take care of the situation where the user only passes 3 params for the point
		// if they screw it up any worse than that, its their own fault.
		fParams[3]= 0.0f;
	default:
		// the next line takes into account the earlier "argc-= 2;"
		if (!ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN-argc), argv+2)) return ERR_PARAMPARSE;
		glLightfv(light, mode, fParams);
	}
	return 0;
}
PUBLISHED(glLightModel, DoLightModel) {
	if (argc < 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	if (iParams[0] == GL_LIGHT_MODEL_AMBIENT) {
		if (!ScanParams("c", argv+1)) return ERR_PARAMPARSE;
	}
	else
		if (!ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+1-argc), argv+1)) return ERR_PARAMPARSE;
	glLightModelfv(iParams[0], fParams);
	return 0;
}
PUBLISHED(glShadeModel, DoShadeModel) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glShadeModel(iParams[0]);
	return 0;
}
PUBLISHED(glMaterial, DoMaterial) {
	int face;
	if (argc <= 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv)) return ERR_PARAMPARSE;
	if (iParams[1] == GL_COLOR_INDEXES) {
		// Handle color indicies as 3 mandatory integers
		face= iParams[0];
		if (argc != 5) return ERR_PARAMCOUNT;
		if (!ScanParams("iii", argv+2)) return ERR_PARAMPARSE;
		glMaterialiv(face, GL_COLOR_INDEXES, iParams);
	}
	else {
		// Handle everything else as a color.
		if (argc != 3 && argc != 5 && argc != 6) return ERR_PARAMCOUNT;
		if (!ScanParams("c", argv+2)) return ERR_PARAMPARSE;
		glMaterialfv(iParams[0], iParams[1], fParams);
	}
	return 0;
}
PUBLISHED(glColorMaterial, DoColorMaterial) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("ii", argv)) return ERR_PARAMPARSE;
	glLightModelf(iParams[0], iParams[1]);
	return 0;
}

//----------------------------------------------------------------------------
// Texture Functions
//
PUBLISHED(glBindTexture, DoBindTexture) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("iT", argv)) return ERR_PARAMPARSE;
	glBindTexture(iParams[0], sParams[0]->Value);
	return 0;
}
PUBLISHED(glTexParameter, DoTexParameter) {
	const int FIXED_PARAMS= 2;
	if (argc <= FIXED_PARAMS || argc >= MAX_GL_PARAMS) return ERR_PARAMCOUNT;
	if (argc == 3)
		if (ScanParams("iii", argv)) {
			glTexParameteri(iParams[0], iParams[1], iParams[2]);
			return 0;
		}
	// 3-integer conversion failed, so maybe it's an array of floats
	if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
		glTexParameterfv(iParams[0], iParams[1], fParams);
	else
		return ERR_PARAMPARSE;
	return 0;
}
PUBLISHED(glTexCoord, DoTexCoord) {
	if (argc >= 1 && argc <= 4) {
		if (!ScanParams(VAR_DBL+(VAR_DBL_LEN-argc), argv)) return ERR_PARAMPARSE;

		switch (argc) {
		case 1: glTexCoord1dv(dParams); break;
		case 2: glTexCoord2dv(dParams); break;
		case 3: glTexCoord3dv(dParams); break;
		case 4: glTexCoord4dv(dParams); break;
		}
	}
	else return ERR_PARAMCOUNT;
	return 0;
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
	if (!ScanParams(VAR_DBL+VAR_DBL_LEN-16, argv)) return ERR_PARAMPARSE;
	glLoadMatrixd(dParams);
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
	if (!ScanParams(VAR_DBL+VAR_DBL_LEN-16, argv)) return ERR_PARAMPARSE;
	glMultMatrixd(dParams);
	return 0;
}
PUBLISHED(glScale, DoScale) {
	if (argc != 3) return ERR_PARAMCOUNT;
	if (!ScanParams("ddd", argv)) return ERR_PARAMPARSE;
	glScaled(dParams[0], dParams[1], dParams[2]);
	return 0;
}
PUBLISHED(glTranslate, DoTranslate) {
	if (argc != 3) return ERR_PARAMCOUNT;
	if (!ScanParams("ddd", argv)) return ERR_PARAMPARSE;
	glTranslated(dParams[0], dParams[1], dParams[2]);
	return 0;
}
PUBLISHED(glRotate, DoRotate) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("dddd", argv)) return ERR_PARAMPARSE;
	glRotated(dParams[0], dParams[1], dParams[2], dParams[3]);
	return 0;
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
PUBLISHED(glViewport, DoViewport) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("iiii", argv)) return ERR_PARAMPARSE;
	glViewport(iParams[0], iParams[1], iParams[2], iParams[3]);
	return 0;
}
PUBLISHED(glOrtho, DoOrtho) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("dddddd", argv)) return ERR_PARAMPARSE;
	glOrtho(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
	return 0;
}
PUBLISHED(glFrustum, DoFrustum) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("dddddd", argv)) return ERR_PARAMPARSE;
	glFrustum(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
	return 0;
}

//----------------------------------------------------------------------------
// Display List Functions
//
PUBLISHED(glNewList, DoNewList) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("Li", argv)) return ERR_PARAMPARSE;
	glNewList(sParams[0]->Value, iParams[0]);
	return 0;
}
PUBLISHED(glEndList, DoEndList) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glEndList();
	return 0;
}
PUBLISHED(glCallList, DoCallList) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("l", argv)) return ERR_PARAMPARSE;
	glCallList(sParams[0]->Value);
	return 0;
}

//----------------------------------------------------------------------------
// Glu Functions
//
PUBLISHED(gluLookAt, DoLookAt) {
	if (argc != 9) return ERR_PARAMCOUNT;
	if (!ScanParams("ddddddddd", argv)) return ERR_PARAMPARSE;
	gluLookAt(dParams[0], dParams[1], dParams[2],
		dParams[3], dParams[4], dParams[5],
		dParams[6], dParams[7], dParams[8]);
	return 0;
}
PUBLISHED(gluNewQuadric, DoNewQuadric) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("Q", argv)) return ERR_PARAMPARSE;
	// nothing to do- ScanParams already created it.
	return 0;
}
PUBLISHED(gluQuadricDrawStyle, DoQuadricDrawStyle) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("qi", argv)) return ERR_PARAMPARSE;
	gluQuadricDrawStyle((GLUquadric*)sParams[0]->Data, iParams[0]);
	return 0;
}
PUBLISHED(gluQuadricNormals, DoQuadricNormals) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("qi", argv)) return ERR_PARAMPARSE;
	gluQuadricNormals((GLUquadric*)sParams[0]->Data, iParams[0]);
	return 0;
}
PUBLISHED(gluQuadricOrientation, DoQuadricOrientation) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("qi", argv)) return ERR_PARAMPARSE;
	gluQuadricOrientation((GLUquadric*)sParams[0]->Data, iParams[0]);
	return 0;
}
PUBLISHED(gluQuadricTexture, DoQuadricTexture) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("qi", argv)) return ERR_PARAMPARSE;
	gluQuadricTexture((GLUquadric*)sParams[0]->Data, iParams[0]);
	return 0;
}
PUBLISHED(gluCylinder, DoCylinder) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("qdddii", argv)) return ERR_PARAMPARSE;
	gluCylinder((GLUquadric*)sParams[0]->Data, dParams[0], dParams[1], dParams[2], iParams[0], iParams[1]);
	return 0;
}
PUBLISHED(gluSphere, DoSphere) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("qdii", argv)) return ERR_PARAMPARSE;
	gluSphere((GLUquadric*)sParams[0]->Data, dParams[0], iParams[0], iParams[1]);
	return 0;
}
PUBLISHED(gluDisk, DoDisk) {
	if (argc != 5) return ERR_PARAMCOUNT;
	if (!ScanParams("qddii", argv)) return ERR_PARAMPARSE;
	gluDisk((GLUquadric*)sParams[0]->Data, dParams[0], dParams[1], iParams[0], iParams[1]);
	return 0;
}
PUBLISHED(gluPartialDisk, DoPartialDisk) {
	if (argc != 7) return ERR_PARAMCOUNT;
	if (!ScanParams("qddiidd", argv)) return ERR_PARAMPARSE;
	gluPartialDisk((GLUquadric*)sParams[0]->Data, dParams[0], dParams[1], iParams[0], iParams[1], dParams[2], dParams[3]);
	return 0;
}


//----------------------------------------------------------------------------
// Glut Functions
//
PUBLISHED(glutIgnoreKeyRepeat, DoIgnoreKeyRepeat) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glutIgnoreKeyRepeat(iParams[0]);
	return 0;
}
PUBLISHED(glutSwapBuffers, DoSwapBuffers) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glutSwapBuffers();
	return 0;
}

//----------------------------------------------------------------------------
// Parsing Functions
//
bool ScanParams(const char* ParamType, char** Args) {
	GLubyte colorVals[4];
	GLint *iParam= iParams;
	GLfloat *fParam= fParams;
	GLdouble *dParam= dParams;
	const SymbVarEntry **sParam= sParams;
	int i;
	bool Success= true;
	while (Success && *ParamType != '\0') {
		switch (*ParamType) {
		// Integer value
		case 'i': Success= ParseInt(*Args, iParam++); break;
		// Float value
		case 'f': Success= ParseFloat(*Args, fParam++); break;
		// Double value
		case 'd': Success= ParseDouble(*Args, dParam++); break;
		// Display list, don't autocreate
		case 'l': Success= ParseSymbVar(*Args, sParam++, false, NAMED_LIST); break;
		// Display list, do autocreate
		case 'L': Success= ParseSymbVar(*Args, sParam++, true, NAMED_LIST); break;
		// Quadric, don't autocreate
		case 'q': Success= ParseSymbVar(*Args, sParam++, false, NAMED_QUADRIC); break;
		// Quadric, do autocreate
		case 'Q': Success= ParseSymbVar(*Args, sParam++, true, NAMED_QUADRIC); break;
		// Texture, don't autocreate
		case 't': Success= ParseSymbVar(*Args, sParam++, false, NAMED_TEXTURE); break;
		// Texture, do autocreate
		case 'T': Success= ParseSymbVar(*Args, sParam++, true, NAMED_TEXTURE); break;
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
		// Catch programming errors
		default:
			fprintf(stderr, "ScanParams: Unknown param type '%c'!  (bug)\n", *ParamType);
			Success= false;
		}
		Args++;
		ParamType++;
	}
	return Success;
}

bool ParseInt(const char* Text, GLint *Result) {
	const IntConstHashEntry *SymConst;
	char *EndPtr;
	
	if (Text[0] == 'G' && Text[1] == 'L' && Text[2] != '\0') {
		SymConst= GetIntConst(Text);
		if (SymConst) {
			*Result= SymConst->Value;
			return true;
		}
		else return false;
	}
	else if (Text[0] == '0' && Text[1] == 'x') {
		*Result= strtol(Text, &EndPtr, 16);
		return (EndPtr != Text);
	}
	else {
		*Result= strtol(Text, &EndPtr, 10);
		return (EndPtr != Text);
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
	return (EndPtr != Text);
}

bool ParseColor(const char* Text, GLubyte *Result) {
	int i, hexval;
	char *StopPos;
	hexval= strtol(Text, &StopPos, 16);
	if (StopPos-Text != 6) {
 		if (StopPos-Text != 8) return false;
		Result[3]= hexval & 0xFF;
		hexval>>= 8;
	}
	else
		Result[3]= 0xFF;
	Result[2]= hexval & 0xFF;
	Result[1]= (hexval>>8) & 0xFF;
	Result[0]= (hexval>>16) & 0xFF;
	return true;
}

bool ParseSymbVar(const char* Text, const SymbVarEntry **Result, bool AutoCreate, int Type) {
	const SymbVarEntry *Entry= GetSymbVar(Text);
	SymbVarEntry *NewEntry; // this exists to avoid the "const pointer" warning.
	
	if (!Entry) {
		if (AutoCreate) {
			NewEntry= CreateSymbVar(Text);
			NewEntry->Type= Type;
			switch (Type) {
			case NAMED_LIST: NewEntry->Value= glGenLists(1); break;
			case NAMED_QUADRIC: NewEntry->Data= gluNewQuadric(); break;
			case NAMED_TEXTURE: glGenTextures(1, &(NewEntry->Value)); break;
			}
			DEBUGMSG(("Created named object %s = %d\n", Text, NewEntry->Value));
			Entry= NewEntry;
		}
		else {
			fprintf(stdout, "Symbolic constant: \"%s\" has not been created.\n", Text);
			return false;
		}
	}
	else if (Entry->Type != Type) {
		fprintf(stdout, "Symbolic constant \"%s\" is not of type %s (it's a %s)\n", Text, SymbVarTypeName[Type], SymbVarTypeName[Entry->Type]);
		return false;
	}
	*Result= Entry;
	return true;
}

