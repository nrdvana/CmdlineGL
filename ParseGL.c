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
}
PUBLISHED(glEnable, DoEnable) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glEnable(iParams[0]);
}
PUBLISHED(glDisable, DoDisable) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glDisable(iParams[0]);
}
PUBLISHED(glClear, DoClear) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glClear(iParams[0]);
}
PUBLISHED(glClearColor, DoClearColor) {
	if (argc == 4)
		if (ScanParams("ffff", argv))
			glClearColor(fParams[0], fParams[1], fParams[2], fParams[3]);
}
PUBLISHED(glClearDepth, DoClearDepth) {
	if (argc == 1)
		if (ScanParams("f", argv))
			glClearDepth(fParams[0]);
}
PUBLISHED(glBegin, DoBegin) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glBegin(iParams[0]);
}
PUBLISHED(glEnd, DoEnd) {
	if (argc == 0)
		glEnd();
}

//----------------------------------------------------------------------------
// Vertex Functions
//
PUBLISHED(glVertex, DoVertex) {
	if (argc == 2) {
		if (ScanParams("dd", argv))
			glVertex2dv(dParams);
	}
	else if (argc == 3) {
		if (ScanParams("ddd", argv))
			glVertex3dv(dParams);
	}
	else if (argc == 4) {
		if (ScanParams("dddd", argv))
			glVertex4dv(dParams);
	}
}
PUBLISHED(glNormal, DoNormal) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glNormal3dv(dParams);
}

//----------------------------------------------------------------------------
// Color Functions
//
PUBLISHED(glColorub, DoColorub) {
	if (argc == 3) {
		if (ScanParams("iii", argv))
			glColor3ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2]);
	}
	else if (argc == 4) {
		if (ScanParams("iiii", argv))
			glColor4ub((GLbyte)iParams[0], (GLbyte)iParams[1], (GLbyte)iParams[2], (GLbyte)iParams[3]);
	}
}
PUBLISHED(glColor, DoColor) {
	if (argc == 3) {
		if (ScanParams("ddd", argv))
			glColor3dv(dParams);
	}
	else if (argc == 4) {
		if (ScanParams("dddd", argv))
			glColor4dv(dParams);
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
	}
}
PUBLISHED(glLightModel, DoLightModel) {
	const int FIXED_PARAMS= 1;
	if (argc > FIXED_PARAMS && argc < MAX_PARAMS) {
		if (ScanParams("i", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glLightModelfv(iParams[0], fParams);
	}
}
PUBLISHED(glMaterial, DoMaterial) {
	const int FIXED_PARAMS= 2;
	if (argc > FIXED_PARAMS && argc < MAX_PARAMS)
		if (ScanParams("ii", argv) && ScanParams(VAR_FLOAT+(VAR_FLOAT_LEN+FIXED_PARAMS-argc), argv+FIXED_PARAMS))
			glMaterialfv(iParams[0], iParams[1], fParams);
}
PUBLISHED(glColorMaterial, DoColorMaterial) {
	if (argc == 2)
		if (ScanParams("ii", argv))
			glLightModelf(iParams[0], iParams[1]);
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
}

//----------------------------------------------------------------------------
// Modelview Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity) {
	if (argc == 0)
		glLoadIdentity();
}
PUBLISHED(glPushMatrix, DoPushMatrix) {
	if (argc == 0)
		glPushMatrix();
}
PUBLISHED(glPopMatrix, DoPopMatrix) {
	if (argc == 0)
		glPopMatrix();
}
PUBLISHED(glScale, DoScale) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glScaledv(dParams);
}
PUBLISHED(glTranslate, DoTranslate) {
	if (argc == 3)
		if (ScanParams("ddd", argv))
			glTranslatedv(dParams);
}
PUBLISHED(glRotate, DoRotate) {
	if (argc == 4)
		if (ScanParams("dddd", argv))
			glRotated(dParams[0], dParams[1], dParams[2], dParams[3]);
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
PUBLISHED(glViewport, DoViewport) {
	if (argc == 4)
		if (ScanParams("iiii", argv))
			glViewport(iParams[0], iParams[1], iParams[2], iParams[3]);
}
PUBLISHED(glOrtho, DoOrtho) {
	if (argc == 6)
		if (ScanParams("dddddd", argv))
			glOrtho(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
}
PUBLISHED(glFrustum, DoFrustum) {
	if (argc == 6)
		if (ScanParams("dddddd", argv))
			glFrustum(dParams[0], dParams[1], dParams[2], dParams[3], dParams[4], dParams[5]);
}

//----------------------------------------------------------------------------
// Display List Functions
//
PUBLISHED(glNewList, DoNewList) {
	if (argc == 2)
		if (ScanParams("ii", argv))
			glNewList(iParams[0], iParams[1]);
}
PUBLISHED(glEndList, DoEndList) {
	if (argc == 0)
		glEndList();
}
PUBLISHED(glCallList, DoCallList) {
	if (argc == 1)
		if (ScanParams("i", argv))
			glCallList(iParams[0]);
}

//----------------------------------------------------------------------------
// Glut Functions
//
PUBLISHED(glutSwapBuffers, DoSwapBuffers) {
	if (argc == 0)
		glutSwapBuffers();
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
		SymConst= GetIntConst(Text+2);
		if (SymConst) {
			*Result= SymConst->Value;
			return true;
		}
		else return false;
	}
	else {
		*Result= strtoi(Text, &EndPtr, 0);
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

