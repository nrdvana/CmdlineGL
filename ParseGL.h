#ifndef PARSEGL_H
#define PARSEGL_H

#include "Global.h"

//----------------------------------------------------------------------------
// Setup Functions
//
PUBLISHED(glMatrixMode, DoMatrixMode);
PUBLISHED(glEnable, DoEnable);
PUBLISHED(glDisable, DoDisable);
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
PUBLISHED(glColorub, DoColorub);
PUBLISHED(glColor, DoColor);

//----------------------------------------------------------------------------
// Lighting Functions
//
PUBLISHED(glLight, DoLight);
PUBLISHED(glLightModel, DoLightModel);
PUBLISHED(glMaterial, DoMaterial);
PUBLISHED(glColorMaterial, DoColorMaterial);

//----------------------------------------------------------------------------
// Texture Functions
//
PUBLISHED(glTexCoord, DoTexCoord);

//----------------------------------------------------------------------------
// Modelview Matrix Functions
//
PUBLISHED(glLoadIdentity, DoLoadIdentity);
PUBLISHED(glPushMatrix, DoPushMatrix);
PUBLISHED(glPopMatrix, DoPopMatrix);
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
// Glut Functions
//
PUBLISHED(glutSwapBuffers, DoSwapBuffers);
PUBLISHED(gluCylinder, DoCylinder);
PUBLISHED(gluSphere, DoSphere);

#endif
