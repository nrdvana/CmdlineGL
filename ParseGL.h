#ifndef PARSEGL_H
#define PARSEGL_H

#include "Global.h"
#include "GlHeaders.h"
#include "SymbolHash.h"

typedef struct ScanParamsResult_t {
	GLint Ints[MAX_GL_PARAMS];
	GLfloat Floats[MAX_GL_PARAMS];
	GLdouble Doubles[MAX_GL_PARAMS];
	const SymbVarEntry* Symbolics[MAX_GL_PARAMS];
	const char* FName; // there can only be one file name, currently
	int ParamsParsed;
	bool Success;
} ScanParamsResult;

bool ScanParams(const char* ParamType, char** Args, ScanParamsResult* Result);

//----------------------------------------------------------------------------
// CmdlineGL Functions
//
PUBLISHED(cglPushDivisor, DoPushDivisor);
PUBLISHED(cglPopDivisor, DoPopDivisor);
PUBLISHED(cglSwapBuffers,DoSwapBuffers);

//----------------------------------------------------------------------------
// Setup Functions
//
PUBLISHED(glMatrixMode, DoMatrixMode);
PUBLISHED(glEnable, DoEnable);
PUBLISHED(glDisable, DoDisable);
PUBLISHED(glHint, DoHint);
PUBLISHED(glClear, DoClear);
PUBLISHED(glClearColor, DoClearColor);
PUBLISHED(glClearDepth, DoClearDepth);
PUBLISHED(glBegin, DoBegin);
PUBLISHED(glEnd, DoEnd);
PUBLISHED(glFlush, DoFlush);

//----------------------------------------------------------------------------
// Vertex Functions
//
PUBLISHED(glVertex, DoVertex);
PUBLISHED(glNormal, DoNormal);

//----------------------------------------------------------------------------
// Color Functions
//
PUBLISHED(glColor, DoColor);
PUBLISHED(glFog, DoFog);
PUBLISHED(glBlendFunc, DoBlendFunc);

//----------------------------------------------------------------------------
// Lighting Functions
//
PUBLISHED(glLight, DoLight);
PUBLISHED(glLightModel, DoLightModel);
PUBLISHED(glShadeModel, DoShadeModel);
PUBLISHED(glMaterial, DoMaterial);
PUBLISHED(glColorMaterial, DoColorMaterial);

//----------------------------------------------------------------------------
// Texture Functions
//
PUBLISHED(glBindTexture, DoBindTexture);
PUBLISHED(glTexParameter, DoTexParameter);
PUBLISHED(glTexCoord, DoTexCoord);
PUBLISHED(cglNewFont, DoNewFont);

//----------------------------------------------------------------------------
// Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity);
PUBLISHED(glLoadMatrix, DoLoadMatrix);
PUBLISHED(glPushMatrix, DoPushMatrix);
PUBLISHED(glPopMatrix, DoPopMatrix);
PUBLISHED(glMultMatrix, DoMultMatrix);
PUBLISHED(glScale, DoScale);
PUBLISHED(glTranslate, DoTranslate);
PUBLISHED(glRotate, DoRotate);

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
PUBLISHED(glViewport, DoViewport);
PUBLISHED(glOrtho, DoOrtho);
PUBLISHED(glFrustum, DoFrustum);

//----------------------------------------------------------------------------
// Display List Functions
//
PUBLISHED(glNewList, DoNewList);
PUBLISHED(glEndList, DoEndList);
PUBLISHED(glCallList, DoCallList);

//----------------------------------------------------------------------------
// glu Functions
//
PUBLISHED(gluLookAt, DoLookAt);
PUBLISHED(gluNewQuadric, DoNewQuadric);
PUBLISHED(gluQuadricDrawStyle, DoQuadricDrawStyle);
PUBLISHED(gluQuadricNormals, DoQuadricNormals);
PUBLISHED(gluQuadricOrientation, DoQuadricOrientation);
PUBLISHED(gluQuadricTexture, DoQuadricTexture);
PUBLISHED(gluCylinder, DoCylinder);
PUBLISHED(gluSphere, DoSphere);
PUBLISHED(gluDisk, DoDisk);
PUBLISHED(gluPartialDisk, DoPartialDisk);

#endif
