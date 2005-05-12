#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "ParseGL.h"
#include "SymbolHash.h"
	
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
int dlistParams[MAX_GL_PARAMS];
GLUquadric* quadricParams[MAX_GL_PARAMS];

bool ScanParams(const char* ParamType, char** Args);
bool ParseInt(const char* Text, GLint *Result);
bool ParseFloat(const char* Text, GLfloat *Result);
bool ParseDouble(const char* Text, GLdouble *Result);
bool ParseNamedDList(const char* Text, int *Result, bool AutoCreate);
bool ParseNamedQuadric(const char* Text, GLUquadric **Result, bool AutoCreate);

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
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glClear(iParams[0]);
	return 0;
}
PUBLISHED(glClearColor, DoClearColor) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("ffff", argv)) return ERR_PARAMPARSE;
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
PUBLISHED(glColorub, DoColorub) {
	if (argc == 3) {
		if (!ScanParams("iii", argv)) return ERR_PARAMPARSE;
		glColor3ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2]);
	}
	else if (argc == 4) {
		if (!ScanParams("iiii", argv)) return ERR_PARAMPARSE;
		glColor4ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2], (GLbyte)iParams[3]);
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glColor, DoColor) {
	if (argc == 3) {
		if (!ScanParams("ddd", argv)) return ERR_PARAMPARSE;
		glColor3dv(dParams);
	}
	else if (argc == 4) {
		if (!ScanParams("dddd", argv)) return ERR_PARAMPARSE;
		glColor4dv(dParams);
	}
	else return ERR_PARAMCOUNT;
	return 0;
}

//----------------------------------------------------------------------------
// Lighting Functions
//
PUBLISHED(glLight, DoLight) {
	const int FIXED_PARAMS= 2;
	if (argc > FIXED_PARAMS && argc < MAX_GL_PARAMS) {
		if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glLightfv(iParams[0], iParams[1], fParams);
		else
			return ERR_PARAMPARSE;
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glLightModel, DoLightModel) {
	const int FIXED_PARAMS= 1;
	if (argc > FIXED_PARAMS && argc < MAX_GL_PARAMS) {
		if (ScanParams("i", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glLightModelfv(iParams[0], fParams);
		else
			return ERR_PARAMPARSE;
	}
	else return ERR_PARAMCOUNT;
	return 0;
}
PUBLISHED(glShadeModel, DoShadeModel) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("i", argv)) return ERR_PARAMPARSE;
	glShadeModel(iParams[0]);
	return 0;
}
PUBLISHED(glMaterial, DoMaterial) {
	const int FIXED_PARAMS= 2;
	if (argc > FIXED_PARAMS && argc < MAX_GL_PARAMS) {
		if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glMaterialfv(iParams[0], iParams[1], fParams);
		else
			return ERR_PARAMPARSE;
	}
	else return ERR_PARAMCOUNT;
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
// Modelview Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity) {
	if (argc != 0) return ERR_PARAMCOUNT;
	glLoadIdentity();
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
	glNewList(dlistParams[0], iParams[0]);
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
	glCallList(dlistParams[0]);
	return 0;
}

//----------------------------------------------------------------------------
// Glu Functions
//
PUBLISHED(gluNewQuadric, DoNewQuadric) {
	if (argc != 1) return ERR_PARAMCOUNT;
	if (!ScanParams("Q", argv)) return ERR_PARAMPARSE;
	// nothing to do- scan params already created it.
	return 0;
}
PUBLISHED(gluQuadricDrawStyle, DoQuadricDrawStyle) {
	if (argc != 2) return ERR_PARAMCOUNT;
	if (!ScanParams("qi", argv)) return ERR_PARAMPARSE;
	gluQuadricDrawStyle(quadricParams[0], iParams[0]);
	return 0;
}
PUBLISHED(gluCylinder, DoCylinder) {
	if (argc != 6) return ERR_PARAMCOUNT;
	if (!ScanParams("qdddii", argv)) return ERR_PARAMPARSE;
	gluCylinder(quadricParams[0], dParams[0], dParams[1], dParams[2], iParams[0], iParams[1]);
	return 0;
}
PUBLISHED(gluSphere, DoSphere) {
	if (argc != 4) return ERR_PARAMCOUNT;
	if (!ScanParams("qdii", argv)) return ERR_PARAMPARSE;
	gluSphere(quadricParams[0], dParams[0], iParams[0], iParams[1]);
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
	GLint *iParam= iParams;
	GLfloat *fParam= fParams;
	GLdouble *dParam= dParams;
	int *dlistParam= dlistParams;
	GLUquadric **quadricParam= quadricParams;
	bool Success= true;
	while (Success && *ParamType != '\0') {
		switch (*ParamType) {
		case 'i': Success= ParseInt(*Args, iParam++); break;
		case 'f': Success= ParseFloat(*Args, fParam++); break;
		case 'd': Success= ParseDouble(*Args, dParam++); break;
		case 'l': Success= ParseNamedDList(*Args, dlistParam++, false); break;
		case 'L': Success= ParseNamedDList(*Args, dlistParam++, true); break;
		case 'q': Success= ParseNamedQuadric(*Args, quadricParam++, false); break;
		case 'Q': Success= ParseNamedQuadric(*Args, quadricParam++, true); break;
		default: Success= false;
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
	char *EndPtr;
	*Result= strtod(Text, &EndPtr);
	return (EndPtr != Text);
}

bool ParseDouble(const char* Text, GLdouble *Result) {
	char *EndPtr;
	*Result= strtod(Text, &EndPtr);
	return (EndPtr != Text);
}

bool ParseNamedDList(const char* Text, int *Result, bool AutoCreate) {
	const SymbVarEntry *Entry= GetSymbVar(Text);
	SymbVarEntry *NewEntry; // this exists to avoid the "const pointer" warning.
	
	if (!Entry && AutoCreate) {
		NewEntry= CreateSymbVar(Text);
		NewEntry->Value= glGenLists(1);
		NewEntry->Type= NAMED_LIST;
		DEBUGMSG(("Created named list %s = %d\n", Text, NewEntry->Value));
		Entry= NewEntry;
	}
	if (Entry && Entry->Type == NAMED_LIST) {
		*Result= Entry->Value;
		return true;
	}
	else return false;
}

bool ParseNamedQuadric(const char* Text, GLUquadric **Result, bool AutoCreate) {
	const SymbVarEntry *Entry= GetSymbVar(Text);
	SymbVarEntry *NewEntry; // this exists to avoid the "const pointer" warning.

	if (!Entry && AutoCreate) {
		NewEntry= CreateSymbVar(Text);
		NewEntry->Value= (int) gluNewQuadric();
		NewEntry->Type= NAMED_QUADRIC;
		DEBUGMSG(("Created quadric %s = %d\n", Text, NewEntry->Value));
		Entry= NewEntry;
	}
	if (Entry && Entry->Type == NAMED_QUADRIC) {
		*Result= (GLUquadric*) Entry->Value;
		return true;
	}
	else return false;
}

