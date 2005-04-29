#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <ctype.h>
#include <stdlib.h>
#include "ParseGL.h"
#include "SymbolHash.h"
	
#define MAX_PARAMS 32

const char* VAR_INT="iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
#define VAR_INT_LEN 32
const char* VAR_FLOAT="ffffffffffffffffffffffffffffffff";
#define VAR_FLOAT_LEN 32
const char* VAR_DBL="dddddddddddddddddddddddddddddddd";
#define VAR_DBL_LEN 32

GLint iParams[MAX_PARAMS];
GLfloat fParams[MAX_PARAMS];
GLdouble dParams[MAX_PARAMS];

bool ScanParams(const char* ParamType, char** Args);
bool ParseInt(const char* Text, GLint *Result);
bool ParseFloat(const char* Text, GLfloat *Result);
bool ParseDouble(const char* Text, GLdouble *Result);

//----------------------------------------------------------------------------
// Setup Functions
//
PUBLISHED(glMatrixMode, DoMatrixMode) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glMatrixMode(iParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glEnable, DoEnable) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glEnable(iParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glDisable, DoDisable) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glDisable(iParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glClear, DoClear) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glClear(iParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glClearColor, DoClearColor) {
	if (argc == 4)
		if (ScanParams("ffff", argv))
			glClearColor(fParams[0], fParams[1], fParams[2], fParams[3]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glClearDepth, DoClearDepth) {
	if (argc == 1)
		if (ScanParams("f", argv))
			glClearDepth(fParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glBegin, DoBegin) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glBegin(iParams[0]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glEnd, DoEnd) {
	if (argc == 0)
		glEnd();
	else
		DEBUGMSG("Err.");
}
PUBLISHED(glFlush, DoFlush) {
	if (argc == 0)
		glFlush();
	else
		DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Vertex Functions
//
PUBLISHED(glVertex, DoVertex) {
	if (argc == 2) {
		if (ScanParams("dd", argv))
			glVertex2dv(dParams);
		else
			DEBUGMSG("Err.");
	}
	else if (argc == 3) {
		if (ScanParams("ddd", argv))
			glVertex3dv(dParams);
		else
			DEBUGMSG("Err.");
	}
	else if (argc == 4) {
		if (ScanParams("dddd", argv))
			glVertex4dv(dParams);
		else
			DEBUGMSG("Err.");
	}
}
PUBLISHED(glNormal, DoNormal) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glNormal3dv(dParams);
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Color Functions
//
PUBLISHED(glColorub, DoColorub) {
	if (argc == 3) {
		if (ScanParams("iii", argv))
			glColor3ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2]);
		else
			DEBUGMSG("Err.");
	}
	else if (argc == 4) {
		if (ScanParams("iiii", argv))
			glColor4ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2], (GLbyte)iParams[3]);
		else
			DEBUGMSG("Err.");
	}
}
PUBLISHED(glColor, DoColor) {
	if (argc == 3) {
		if (ScanParams("ddd", argv))
			glColor3dv(dParams);
		else
			DEBUGMSG("Err.");
	}
	else if (argc == 4) {
		if (ScanParams("dddd", argv))
			glColor4dv(dParams);
		else
			DEBUGMSG("Err.");
	}
}

//----------------------------------------------------------------------------
// Lighting Functions
//
PUBLISHED(glLight, DoLight) {
	const int FIXED_PARAMS= 2;
	if (argc > FIXED_PARAMS && argc < MAX_PARAMS) {
		if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glLightfv(iParams[0], iParams[1], fParams);
		else
			DEBUGMSG("Err.");
	}
}
PUBLISHED(glLightModel, DoLightModel) {
	const int FIXED_PARAMS= 1;
	if (argc > FIXED_PARAMS && argc < MAX_PARAMS) {
		if (ScanParams("i", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glLightModelfv(iParams[0], fParams);
		else
			DEBUGMSG("Err.");
	}
}
PUBLISHED(glMaterial, DoMaterial) {
	const int FIXED_PARAMS= 2;
	if (argc > FIXED_PARAMS && argc < MAX_PARAMS)
		if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glMaterialfv(iParams[0], iParams[1], fParams);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glColorMaterial, DoColorMaterial) {
	if (argc == 2)
		if (ScanParams("ii", argv))
			glLightModelf(iParams[0], iParams[1]);
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Texture Functions
//
PUBLISHED(glTexCoord, DoTexCoord) {
	if (argc >= 1 && argc <= 4)
		if (ScanParams(VAR_DBL+(VAR_DBL_LEN-argc), argv)) {
			switch (argc) {
			case 1: glTexCoord1dv(dParams); break;
			case 2: glTexCoord2dv(dParams); break;
			case 3: glTexCoord3dv(dParams); break;
			case 4: glTexCoord4dv(dParams); break;
			}
		}
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Modelview Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity) {
	if (argc == 0)
		glLoadIdentity();
	else
		DEBUGMSG("Err.");
}
PUBLISHED(glPushMatrix, DoPushMatrix) {
	if (argc == 0)
		glPushMatrix();
	else
		DEBUGMSG("Err.");
}
PUBLISHED(glPopMatrix, DoPopMatrix) {
	if (argc == 0)
		glPopMatrix();
	else
		DEBUGMSG("Err.");
}
PUBLISHED(glScale, DoScale) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glScaled(dParams[0], dParams[1], dParams[2]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glTranslate, DoTranslate) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glTranslated(dParams[0], dParams[1], dParams[2]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glRotate, DoRotate) {
	if (argc == 4)
		if (ScanParams("dddd", argv))
			glRotated(dParams[0], dParams[1], dParams[2], dParams[3]);
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
PUBLISHED(glViewport, DoViewport) {
	if (argc == 4)
		if (ScanParams("iiii", argv))
			glViewport(iParams[0], iParams[1], iParams[2], iParams[3]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glOrtho, DoOrtho) {
	if (argc == 6)
		if (ScanParams("dddddd", argv))
			glOrtho(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glFrustum, DoFrustum) {
	if (argc == 6)
		if (ScanParams("dddddd", argv))
			glFrustum(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Display List Functions
//
PUBLISHED(glNewList, DoNewList) {
	if (argc == 2)
		if (ScanParams("ii", argv))
			glNewList(iParams[0], iParams[1]);
		else
			DEBUGMSG("Err.");
}
PUBLISHED(glEndList, DoEndList) {
	if (argc == 0)
		glEndList();
	else
		DEBUGMSG("Err.");
}
PUBLISHED(glCallList, DoCallList) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glCallList(iParams[0]);
		else
			DEBUGMSG("Err.");
}

//----------------------------------------------------------------------------
// Glut Functions
//
PUBLISHED(glutSwapBuffers, DoSwapBuffers) {
	if (argc == 0)
		glutSwapBuffers();
	else
		DEBUGMSG("Err.");
}

PUBLISHED(gluCylinder, DoCylinder) {
//	if (argc == 0)
}
PUBLISHED(gluSphere, DoSphere) {
}

//----------------------------------------------------------------------------
// Parsing Functions
//
bool ScanParams(const char* ParamType, char** Args) {
	GLint *iParam= iParams;
	GLfloat *fParam= fParams;
	GLdouble *dParam= dParams;
	bool Success= true;
	while (Success && *ParamType != '\0') {
		switch (*ParamType) {
		case 'i': Success= ParseInt(*Args, iParam++); break;
		case 'f': Success= ParseFloat(*Args, fParam++); break;
		case 'd': Success= ParseDouble(*Args, dParam++); break;
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

