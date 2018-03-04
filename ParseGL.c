#include "Global.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <assert.h>
#include "GlHeaders.h"
#include "ParseGL.h"
#include "Server.h"
#include "SymbolHash.h"
#include "ImageLoader.h"
#include "Font.h"

bool PointsInProgress= false; // Whenever glBegin is active, until glEnd
bool FrameInProgress= false;  // True after any gl command, until cglSwapBuffers

//----------------------------------------------------------------------------
// Setup Functions
//
COMMAND(glMatrixMode, "i") {
	glMatrixMode((GLint) parsed->ints[0]);
	return true;
}
COMMAND(glEnable, "i*") {
	while (parsed->iCnt > 0)
		glEnable(parsed->ints[--parsed->iCnt]);
	return true;
}
COMMAND(glDisable, "i*") {
	while (parsed->iCnt > 0)
		glDisable(parsed->ints[--parsed->iCnt]);
	return true;
}
COMMAND(glHint, "ii") {
	glHint(parsed->ints[0], parsed->ints[1]);
	return true;
}
COMMAND(glClear, "i*") {
	int flags= 0;
	while (parsed->iCnt > 0)
		flags|= parsed->ints[--parsed->iCnt];
	glClear(flags);
	FrameInProgress= true;
	return true;
}
COMMAND(glClearColor, "c") {
	glClearColor(parsed->floats[0], parsed->floats[1], parsed->floats[2], parsed->floats[3]);
	return true;
}
COMMAND(glClearDepth, "d") {
	glClearDepth(parsed->doubles[0]);
	return true;
}
COMMAND(glBegin, "i") {
	if (PointsInProgress) {
		parsed->errmsg= "multiple calls to glBegin without glEnd";
		return false;
	}
	glBegin(parsed->ints[0]);
	PointsInProgress= true;
	FrameInProgress= true;
	return true;
}
COMMAND(glEnd, "") {
	if (!PointsInProgress) {
		parsed->errmsg= "glEnd without glBegin";
		return false;
	}
	glEnd();
	PointsInProgress= false;
	return true;
}
COMMAND(glFlush, "") {
	glFlush();
	return true;
}

//----------------------------------------------------------------------------
// Vertex Functions
//
COMMAND(glVertex, "ddd?d?") {
	switch (parsed->dCnt) {
	case 2: glVertex2dv(parsed->doubles); break;
	case 3: glVertex3dv(parsed->doubles); break;
	case 4: glVertex4dv(parsed->doubles); break;
	default:
		return false;
	}
	return true;
}
COMMAND(glNormal, "ddd") {
	glNormal3dv(parsed->doubles);
	return true;
}

//----------------------------------------------------------------------------
// Color Functions
//
COMMAND(glColor, "c") {
	glColor4fv(parsed->floats);
	return true;
}
COMMAND(glFog, "ib") {
	int mode= parsed->ints[0];

	// The parameter to this one really matters, since floats get fixed-point
	//  multiplied, and colors need special treatment.
	switch (mode) {
	case GL_FOG_MODE:
	case GL_FOG_INDEX:
		if (!ParseParams(&parsed->strings[0], "i", parsed))
			return false;
		glFogi(mode, parsed->ints[1]);
		return true;
	case GL_FOG_DENSITY:
	case GL_FOG_START:
	case GL_FOG_END:
		if (!ParseParams(&parsed->strings[0], "f", parsed))
			return false;
		glFogf(mode, parsed->floats[0]);
		return true;
	case GL_FOG_COLOR:
		if (!ParseParams(&parsed->strings[0], "c", parsed))
			return false;
		glFogfv(mode, parsed->floats);
		return true;
	default:
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unsupported mode constant for glFog: %d", mode);
		parsed->errmsg= parsed->errmsg_buf;
	}
	return false;
}
COMMAND(glLight, "iib") {
	int light= parsed->ints[0], pname= parsed->ints[1];

	switch (pname) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
		if (!ParseParams(&parsed->strings[0], "c", parsed))
			return false;
		assert(parsed->fCnt == 4);
		break;
	case GL_POSITION:
	#ifdef GL_SPOT_DIRECTION
	case GL_SPOT_DIRECTION:
	#endif
		if (!ParseParams(&parsed->strings[0], "ffff?", parsed))
			return false;
		assert(parsed->fCnt >= 3 && parsed->fCnt <= 4);
		break;
	#ifdef GL_SPOT_EXPONENT
	case GL_SPOT_EXPONENT:
	case GL_SPOT_CUTOFF:
	#endif
	#ifdef GL_CONSTANT_ATTENUATION
	case GL_CONSTANT_ATTENUATION:
	case GL_LINEAR_ATTENUATION:
	case GL_QUADRATIC_ATTENUATION:
	#endif
		if (!ParseParams(&parsed->strings[0], "f", parsed))
			return false;
		assert(parsed->fCnt == 1);
		break;
	default:
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unsupported pname constant for glLight: %d", pname);
		parsed->errmsg= parsed->errmsg_buf;
		return false;
	}
	glLightfv(light, pname, parsed->floats);
	return true;
}
COMMAND(glLightModel, "ib") {
	int pname= parsed->ints[0];

	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		if (!ParseParams(&parsed->strings[0], "c", parsed))
			return false;
		glLightModelfv(pname, parsed->floats);
		return true;
	#ifdef GL_LIGHT_MODEL_COLOR_CONTROL
	case GL_LIGHT_MODEL_COLOR_CONTROL:
	#endif
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
	case GL_LIGHT_MODEL_TWO_SIDE:
		if (!ParseParams(&parsed->strings[0], "i", parsed))
			return false;
		glLightModeliv(pname, parsed->ints+1);
		return true;
	default:
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unsupported pname constant for glLightModel: %d", pname);
		parsed->errmsg= parsed->errmsg_buf;
	}
	return false;
}
COMMAND(glShadeModel, "i") {
	glShadeModel(parsed->ints[0]);
	return true;
}
COMMAND(glMaterial, "iib") {
	int face= parsed->ints[0], pname= parsed->ints[1];
	
	switch (pname) {
	case GL_COLOR_INDEXES:
		if (!ParseParams(&parsed->strings[0], "iii", parsed))
			return false;
		glMaterialiv(face, pname, parsed->ints+2);
		return true;
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_AMBIENT_AND_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
		if (!ParseParams(&parsed->strings[0], "c", parsed))
			return false;
		glMaterialfv(face, pname, parsed->floats);
		return true;
	case GL_SHININESS:
		if (!ParseParams(&parsed->strings[0], "f", parsed))
			return false;
		glMaterialfv(face, pname, parsed->floats);
		return true;
	default:
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unsupported pname constant for glMaterial: %d", pname);
		parsed->errmsg= parsed->errmsg_buf;
	}
	return false;
}
COMMAND(glColorMaterial, "ii") {
	glColorMaterial(parsed->ints[0], parsed->ints[1]);
	return true;
}
COMMAND(glBlendFunc, "ii") {
	glBlendFunc(parsed->ints[0], parsed->ints[1]);
	return true;
}

//----------------------------------------------------------------------------
// Texture Functions
//
COMMAND(glBindTexture, "iT!") {
	glBindTexture(parsed->ints[0], parsed->objects[0]->Value);
	return true;
}
COMMAND(glTexParameter, "iib") {
	int target= parsed->ints[0], pname= parsed->ints[1];
	
	switch (pname) {
	/* one enum */
	case GL_TEXTURE_MIN_FILTER:
	case GL_TEXTURE_MAG_FILTER:
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	case GL_TEXTURE_WRAP_R:
	#ifdef GL_TEXTURE_COMPARE_MODE
	case GL_TEXTURE_COMPARE_MODE:
	case GL_TEXTURE_COMPARE_FUNC:
	case GL_DEPTH_TEXTURE_MODE:
	case GL_GENERATE_MIPMAP:
	#endif
	/* one integer */
	case GL_TEXTURE_BASE_LEVEL:
	case GL_TEXTURE_MAX_LEVEL:
		if (!ParseParams(&parsed->strings[0], "i", parsed))
			return false;
		glTexParameteri(target, pname, parsed->ints[2]);
		return true;

	/* one float */
	#ifdef GL_TEXTURE_MIN_LOD
	case GL_TEXTURE_MIN_LOD:
	case GL_TEXTURE_MAX_LOD:
	#endif
	case GL_TEXTURE_PRIORITY:
		if (!ParseParams(&parsed->strings[0], "f", parsed))
			return false;
		glTexParameterf(target, pname, parsed->floats[0]);
		return true;

	/* one color */
	case GL_TEXTURE_BORDER_COLOR:
		if (!ParseParams(&parsed->strings[0], "c", parsed))
			return false;
		glTexParameterfv(target, pname, parsed->floats);
		return true;
	default:
		snprintf(parsed->errmsg_buf, sizeof(parsed->errmsg_buf), "Unsupported pname constant for glTexparameter: %d", pname);
		parsed->errmsg= parsed->errmsg_buf;
	}
	return false;
}
COMMAND(glTexCoord, "dd?d?d?") {
	assert( parsed->dCnt >= 1 && parsed->dCnt <= 4 );
	switch (parsed->dCnt) {
	case 1: glTexCoord1dv(parsed->doubles); break;
	case 2: glTexCoord2dv(parsed->doubles); break;
	case 3: glTexCoord3dv(parsed->doubles); break;
	case 4: glTexCoord4dv(parsed->doubles); break;
	}
	return true;
}

//----------------------------------------------------------------------------
// Matrix Functions
//
COMMAND(glLoadIdentity, "") {
	glLoadIdentity();
	return true;
}
COMMAND(glLoadMatrix, "dddddddddddddddd") {
	glLoadMatrixd(parsed->doubles);
	return true;
}
COMMAND(glPushMatrix, "") {
	glPushMatrix();
	return true;
}
COMMAND(glPopMatrix, "") {
	glPopMatrix();
	return true;
}
COMMAND(glMultMatrix, "dddddddddddddddd") {
	glMultMatrixd(parsed->doubles);
	return true;
}
COMMAND(glScale, "dd?d?") {
	assert( parsed->dCnt >= 1 && parsed->dCnt <= 3 );
	if (parsed->dCnt == 1)
		parsed->doubles[2]= parsed->doubles[1]= parsed->doubles[0];
	else if (parsed->dCnt == 2)
		parsed->doubles[2]= 1.0;
	glScaled(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2]);
	return true;
}
COMMAND(glTranslate, "ddd?") {
	assert( parsed->dCnt >= 2 && parsed->dCnt <= 3 );
	if (parsed->dCnt == 2) parsed->doubles[2]= 0.0;
	glTranslated(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2]);
	return true;
}
COMMAND(glRotate, "dddd") {
	glRotated(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2], parsed->doubles[3]);
	return true;
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
COMMAND(glViewport, "iiii") {
	glViewport(parsed->ints[0], parsed->ints[1], parsed->ints[2], parsed->ints[3]);
	return true;
}
COMMAND(glOrtho, "dddddd") {
	glOrtho(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2], parsed->doubles[3], parsed->doubles[4], parsed->doubles[5]);
	return true;
}
COMMAND(glFrustum, "dddddd") {
	glFrustum(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2], parsed->doubles[3], parsed->doubles[4], parsed->doubles[5]);
	return true;
}

//----------------------------------------------------------------------------
// Display List Functions
//
COMMAND(glNewList, "L!i") {
	glNewList(parsed->objects[0]->Value, parsed->ints[0]);
	return true;
}
COMMAND(glEndList, "") {
	glEndList();
	return true;
}
COMMAND(glCallList, "L") {
	glCallList(parsed->objects[0]->Value);
	return true;
}

//----------------------------------------------------------------------------
// Glu Functions
//
COMMAND(gluLookAt, "ddddddddd") {
	gluLookAt(
		parsed->doubles[0], parsed->doubles[1], parsed->doubles[2],
		parsed->doubles[3], parsed->doubles[4], parsed->doubles[5],
		parsed->doubles[6], parsed->doubles[7], parsed->doubles[8]
	);
	return true;
}
COMMAND(gluNewQuadric, "Q!") {
	/* auto-create handled by ParseParams */
	return true;
}
COMMAND(gluQuadricDrawStyle, "Qi") {
	gluQuadricDrawStyle((GLUquadric*) parsed->objects[0]->Data, parsed->ints[0]);
	return true;
}
COMMAND(gluQuadricNormals, "Qi") {
	gluQuadricNormals((GLUquadric*) parsed->objects[0]->Data, parsed->ints[0]);
	return true;
}
COMMAND(gluQuadricOrientation, "Qi") {
	gluQuadricOrientation((GLUquadric*) parsed->objects[0]->Data, parsed->ints[0]);
	return true;
}
COMMAND(gluQuadricTexture, "Qi") {
	gluQuadricTexture((GLUquadric*) parsed->objects[0]->Data, parsed->ints[0]);
	return true;
}
COMMAND(gluCylinder, "Qdddii") {
	gluCylinder((GLUquadric*) parsed->objects[0]->Data,
		parsed->doubles[0], parsed->doubles[1], parsed->doubles[2],
		parsed->ints[0], parsed->ints[1]);
	FrameInProgress= true;
	return true;
}
COMMAND(gluSphere, "Qdii") {
	gluSphere((GLUquadric*) parsed->objects[0]->Data, parsed->doubles[0],
		parsed->ints[0], parsed->ints[1]);
	FrameInProgress= true;
	return true;
}
COMMAND(gluDisk, "Qddii") {
	gluDisk((GLUquadric*) parsed->objects[0]->Data, parsed->doubles[0], parsed->doubles[1],
		parsed->ints[0], parsed->ints[1]);
	FrameInProgress= true;
	return true;
}
COMMAND(gluPartialDisk, "Qddiidd") {
	gluPartialDisk((GLUquadric*) parsed->objects[0]->Data, parsed->doubles[0], parsed->doubles[1],
		parsed->ints[0], parsed->ints[1], parsed->doubles[2], parsed->doubles[3]);
	FrameInProgress= true;
	return true;
}
