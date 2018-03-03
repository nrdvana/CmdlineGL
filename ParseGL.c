#include "Global.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
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
	glMatrixMode((GLint) argv[0].as_long);
	return true;
}
COMMAND(glEnable, "i*") {
	int i;
	for (i=0; i<argc; i++)
		glEnable(argv[i].as_long);
	return true;
}
COMMAND(glDisable, "i*") {
	int i;
	for (i=0; i<argc; i++)
		glDisable(argv[i].as_long);
	return true;
}
COMMAND(glHint, "ii") {
	glHint(argv[0].as_long, argv[1].as_long);
	return true;
}
COMMAND(glClear, "i*") {
	int flags= 0;
	while (argc--)
		flags|= argv[argc].as_long;
	glClear(flags);
	FrameInProgress= true;
	return true;
}
COMMAND(glClearColor, "c") {
	glClearColor(argv[0].as_color[0], argv[0].as_color[1], argv[0].as_color[2], argv[0].as_color[3]);
	return true;
}
COMMAND(glClearDepth, "f") {
	glClearDepth(argv[0].as_double);
	return true;
}
COMMAND(glBegin, "i") {
	if (PointsInProgress) {
		fprintf(stderr, "Error: multiple calls to glBegin without glEnd\n");
		return false;
	}
	glBegin(argv[0].as_long);
	PointsInProgress= true;
	FrameInProgress= true;
	return true;
}
COMMAND(glEnd, "") {
	if (!PointsInProgress) {
		fprintf(stderr, "Error: glEnd without glBegin\n");
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
	switch (argc) {
	case 2: glVertex2d(argv[0].as_double, argv[1].as_double); break;
	case 3: glVertex3d(argv[0].as_double, argv[1].as_double, argv[2].as_double); break;
	case 4: glVertex4d(argv[0].as_double, argv[1].as_double, argv[2].as_double, argv[3].as_double); break;
	default:
		return false;
	}
	return true;
}
COMMAND(glNormal, "ddd") {
	glNormal3d(argv[0].as_double, argv[1].as_double, argv[2].as_double);
	return true;
}

//----------------------------------------------------------------------------
// Color Functions
//
COMMAND(glColor, "c") {
	glColor4fv(argv[0].as_color);
	return true;
}
COMMAND(glFog, "ib") {
	int mode= argv[0].as_long, n_params= 1;
	ParamUnion pTemp[1];

	// The parameter to this one really matters, since floats get fixed-point
	//  multiplied, and colors need special treatment.
	switch (mode) {
	case GL_FOG_MODE:
	case GL_FOG_INDEX:
		if (!ParseParams("glFog", argv[1].as_str, "i", pTemp, &n_params))
			return false;
		glFogi(mode, pTemp[0].as_long);
		return true;
	case GL_FOG_DENSITY:
	case GL_FOG_START:
	case GL_FOG_END:
		if (!ParseParams("glFog", argv[1].as_str, "f", pTemp, &n_params))
			return false;
		glFogf(mode, pTemp[0].as_double);
		return true;
	case GL_FOG_COLOR:
		if (!ParseParams("glFog", argv[1].as_str, "c", pTemp, &n_params))
			return false;
		glFogfv(mode, pTemp[0].as_color);
		return true;
	default:
		fprintf(stderr, "Unsupported mode constant for glFog: %d", mode);
		fflush(stderr);
	}
	return false;
}
COMMAND(glLight, "iib") {
	int mode= argv[0].as_long, light= argv[1].as_long, n_params= 4, i;
	ParamUnion pTemp[4];
	float fTemp[4];

	switch (mode) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
		if (!ParseParams("glLight", argv[2].as_str, "c", pTemp, &n_params))
			return false;
		glLightfv(light, mode, pTemp[0].as_color);
		return true;
	case GL_POSITION:
	#ifdef GL_SPOT_DIRECTION
	case GL_SPOT_DIRECTION:
	#endif
		if (!ParseParams("glLight", argv[2].as_str, "ffff*", pTemp, &n_params))
			return false;
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
		if (!ParseParams("glLight", argv[2].as_str, "f", pTemp, &n_params))
			return false;
	default:
		fprintf(stderr, "Unsupported mode constant for glLight: %d", mode);
		fflush(stderr);
		return false;
	}
	for (i= 0; i < 4; i++)
		fTemp[i]= i < n_params? pTemp[i].as_double : 0.0;
	glLightfv(light, mode, fTemp);
	return true;
}
COMMAND(glLightModel, "ib") {
	ParamUnion pTemp[4];
	GLint iTemp[1];
	int pname= argv[0].as_long, n_params= 4;

	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		if (!ParseParams("glLightModel", argv[1].as_str, "c", pTemp, &n_params))
			return false;
		glLightModelfv(pname, pTemp[0].as_color);
		return true;
	#ifdef GL_LIGHT_MODEL_COLOR_CONTROL
	case GL_LIGHT_MODEL_COLOR_CONTROL:
	#endif
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
	case GL_LIGHT_MODEL_TWO_SIDE:
		if (!ParseParams("glLightModel", argv[1].as_str, "i", pTemp, &n_params))
			return false;
		iTemp[0]= pTemp[0].as_long;
		glLightModeliv(pname, iTemp);
		return true;
	default:
		fprintf(stderr, "Unsupported pname constant for glLightModel: %d", pname);
		fflush(stderr);
	}
	return false;
}
COMMAND(glShadeModel, "i") {
	glShadeModel(argv[0].as_long);
	return true;
}
COMMAND(glMaterial, "iib") {
	GLint iTemp[4];
	GLfloat fTemp[4];
	ParamUnion pTemp[4];
	int face= argv[0].as_long, pname= argv[1].as_long, n_params= 4, i;
	
	switch (pname) {
	case GL_COLOR_INDEXES:
		if (!ParseParams("glMaterial", argv[2].as_str, "iii", pTemp, &n_params))
			return false;
		for (i= 0; i < 3; i++) iTemp[i]= pTemp[i].as_long;
		glMaterialiv(face, pname, iTemp);
		return true;
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_AMBIENT_AND_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
		if (!ParseParams("glMaterial", argv[2].as_str, "c", pTemp, &n_params))
			return false;
		glMaterialfv(face, pname, pTemp[0].as_color);
		return true;
	case GL_SHININESS:
		if (!ParseParams("glMaterial", argv[2].as_str, "f", pTemp, &n_params))
			return false;
		fTemp[0]= pTemp[0].as_double;
		glMaterialfv(face, pname, fTemp);
		return true;
	default:
		fprintf(stderr, "Unsupported pname constant for glMaterial: %d", pname);
		fflush(stderr);
	}
	return 0;
}
COMMAND(glColorMaterial, "ii") {
	glColorMaterial(argv[0].as_long, argv[1].as_long);
	return true;
}
COMMAND(glBlendFunc, "ii") {
	glBlendFunc(argv[0].as_long, argv[1].as_long);
	return true;
}

//----------------------------------------------------------------------------
// Texture Functions
//
COMMAND(glBindTexture, "iT!") {
	glBindTexture(argv[0].as_long, argv[1].as_sym->Value);
	return true;
}
COMMAND(glTexParameter, "iib") {
	GLint iTemp[4];
	GLfloat fTemp[4];
	ParamUnion pTemp[4];
	int target= argv[0].as_long, pname= argv[1].as_long, n_params= 4;
	
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
		if (!ParseParams("glTexParameter", argv[2].as_str, "i", pTemp, &n_params))
			return false;
		glTexParameteri(target, pname, pTemp->as_long);
		return true;

	/* one float */
	#ifdef GL_TEXTURE_MIN_LOD
	case GL_TEXTURE_MIN_LOD:
	case GL_TEXTURE_MAX_LOD:
	#endif
	case GL_TEXTURE_PRIORITY:
		if (!ParseParams("glTexParameter", argv[2].as_str, "f", pTemp, &n_params))
			return false;
		glTexParameterf(target, pname, pTemp->as_double);
		return true;

	/* one color */
	case GL_TEXTURE_BORDER_COLOR:
		if (!ParseParams("glTexParameter", argv[2].as_str, "c", pTemp, &n_params))
			return false;
		glTexParameterfv(target, pname, pTemp->as_color);
		return true;
	default:
		fprintf(stderr, "Unsupported pname constant for glTexparameter: %d", pname);
		fflush(stderr);
	}
	return false;
}
COMMAND(glTexCoord, "dd*") {
	GLdouble dTemp[4];
	int i;
	for (i= 0; i < 4 && i < argc; i++)
		dTemp[i]= argv[i].as_double;
	switch (argc) {
	case 1: glTexCoord1dv(dTemp); break;
	case 2: glTexCoord2dv(dTemp); break;
	case 3: glTexCoord3dv(dTemp); break;
	case 4: glTexCoord4dv(dTemp); break;
	default:
		fprintf(stderr, "incorrect number of arguments to glTexCoord");
		fflush(stderr);
		return false;
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
	GLdouble dTemp[16];
	int i;
	for (i= 0; i < 16; i++)
		dTemp[i]= argv[i].as_double;
	glLoadMatrixd(dTemp);
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
	GLdouble dTemp[16];
	int i;
	for (i= 0; i < 16; i++)
		dTemp[i]= argv[i].as_double;
	glMultMatrixd(dTemp);
	return true;
}
COMMAND(glScale, "dd*") {
	switch (argc) {
	case 3: glScaled(argv[0].as_double, argv[1].as_double, argv[2].as_double); break;
	case 2: glScaled(argv[0].as_double, argv[1].as_double, 1); break;
	case 1: glScaled(argv[0].as_double, argv[0].as_double, argv[0].as_double); break;
	default:
		fprintf(stderr, "incorrect number of arguments to glTexCoord");
		fflush(stderr);
		return false;
	}
	return true;
}
COMMAND(glTranslate, "ddd*") {
	if (argc == 3) {
		glTranslated(argv[0].as_double, argv[1].as_double, argv[2].as_double);
	}
	else if (argc == 2) {
		glTranslated(argv[0].as_double, argv[1].as_double, 0);
	}
	else return false;
	return true;
}
COMMAND(glRotate, "dddd") {
	glRotated(argv[0].as_double, argv[1].as_double, argv[2].as_double, argv[3].as_double);
	return true;
}

//----------------------------------------------------------------------------
// Projectionview Matrix Functions
//
COMMAND(glViewport, "iiii") {
	glViewport(argv[0].as_long, argv[1].as_long, argv[2].as_long, argv[3].as_long);
	return true;
}
COMMAND(glOrtho, "dddddd") {
	glOrtho(argv[0].as_double, argv[1].as_double, argv[2].as_double, argv[3].as_double, argv[4].as_double, argv[5].as_double);
	return true;
}
COMMAND(glFrustum, "dddddd") {
	glOrtho(argv[0].as_double, argv[1].as_double, argv[2].as_double, argv[3].as_double, argv[4].as_double, argv[5].as_double);
	return true;
}

//----------------------------------------------------------------------------
// Display List Functions
//
COMMAND(glNewList, "L!i") {
	glNewList(argv[0].as_sym->Value, argv[1].as_long);
	return true;
}
COMMAND(glEndList, "") {
	glEndList();
	return true;
}
COMMAND(glCallList, "L") {
	glCallList(argv[0].as_sym->Value);
	return true;
}

//----------------------------------------------------------------------------
// Glu Functions
//
COMMAND(gluLookAt, "ddddddddd") {
	gluLookAt(
		argv[0].as_double, argv[1].as_double, argv[2].as_double,
		argv[3].as_double, argv[4].as_double, argv[5].as_double,
		argv[6].as_double, argv[7].as_double, argv[8].as_double
	);
	return true;
}
COMMAND(gluNewQuadric, "Q!") {
	return true;
}
COMMAND(gluQuadricDrawStyle, "Qi") {
	gluQuadricDrawStyle((GLUquadric*)argv[0].as_sym->Data, argv[1].as_long);
	return true;
}
COMMAND(gluQuadricNormals, "Qi") {
	gluQuadricNormals((GLUquadric*)argv[0].as_sym->Data, argv[1].as_long);
	return true;
}
COMMAND(gluQuadricOrientation, "Qi") {
	gluQuadricOrientation((GLUquadric*)argv[0].as_sym->Data, argv[1].as_long);
	return true;
}
COMMAND(gluQuadricTexture, "Qi") {
	gluQuadricTexture((GLUquadric*)argv[0].as_sym->Data, argv[1].as_long);
	return true;
}
COMMAND(gluCylinder, "Qdddii") {
	gluCylinder((GLUquadric*)argv[0].as_sym->Data,
		argv[1].as_double, argv[2].as_double, argv[3].as_double,
		argv[4].as_long, argv[5].as_long);
	FrameInProgress= true;
	return true;
}
COMMAND(gluSphere, "Qdii") {
	gluSphere((GLUquadric*)argv[0].as_sym->Data, argv[1].as_double, argv[2].as_long, argv[3].as_long);
	FrameInProgress= true;
	return true;
}
COMMAND(gluDisk, "Qddii") {
	gluDisk((GLUquadric*)argv[0].as_sym->Data, argv[1].as_double, argv[2].as_double, argv[3].as_long, argv[4].as_long);
	FrameInProgress= true;
	return true;
}
COMMAND(gluPartialDisk, "Qddiidd") {
	gluPartialDisk((GLUquadric*)argv[0].as_sym->Data, argv[1].as_double, argv[2].as_double,
		argv[3].as_long, argv[4].as_long, argv[5].as_double, argv[6].as_double);
	FrameInProgress= true;
	return true;
}
