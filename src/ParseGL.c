#define INCLUDE_GL
#include <config.h>
#include "Global.h"
#include "ParseGL.h"
#include "Server.h"
#include "SymbolHash.h"
#include "ImageLoader.h"
#include "Font.h"

bool PointsInProgress= false; // Whenever glBegin is active, until glEnd
bool FrameInProgress= false;  // True after any gl command, until cglSwapBuffers

/*----------------------------------------------------------------------------
=head2 Setup Functions

=over

=item glEnable FEATURE [FEATURE...]

Enable one or more GL feature bits.

=item glDisable FEATURE [FEATURE...]

Disable one or more GL feature bits.

=item glHint TARGET MODE

Adjust GL optional behavior.

=cut
*/

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

/*----------------------------------------------------------------------------
=head2 Texture Functions

=back
*/

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

/*----------------------------------------------------------------------------
=head2 Matrix Functions

=over

=item glMatrixMode MODE

Select which matrix will be changed by other matrix functions.

=item glLoadIdentity

Reset the matrix to the identity

=item glLoadMatrix I1 I2 I3 I4 J1 J2 J3 J4 K1 K2 K3 K4 L1 L2 L3 L4

Directly overwrite the matrix with 16 values.

=item glPushMatrix

Save a copy of the current matrix.

=item glPopMatrix

Restore the previous saved matrix.

=item glMultMatrix I1 I2 I3 I4 J1 J2 J3 J4 K1 K2 K3 K4 L1 L2 L3 L4

Multiply the current matrix by this specified matrix

=item glScale SCALE [SCALE_Y [SCALE_Z]]

When given one argument, scale the X, Y, and Z axis by the specified value.
When given two arguments, scale X and Y, leaving Z unchanged.
When given three arguments,scale X, Y, and Z.

=item glTranslate X Y [Z]

Apply a translation to the matrix.  The Z coordinate is optional and
defaults to 0.

=item glRotate DEGREES X Y Z

Rotate DEGREES around the axis defined by (X,Y,Z)

=item glViewport X Y WIDTH HEIGHT

Define the 2D region of the screen to be rendered by the current matrix.

=item glOrtho

Set up a projection matrix that maps the given coordinate values to the
edges of the viewport, and sets the near and far clipping plane.

=item glFrustum

Set up a projection matrix where the given coordinates are the edges of the
screen B<at> the near clipping plane, and scale proportionally as the Z
coordinate gets farther from the near plane.

=back
*/

COMMAND(glMatrixMode, "i") {
	glMatrixMode((GLint) parsed->ints[0]);
	return true;
}
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

COMMAND(glViewport, "iiii") {
	glViewport(parsed->ints[0], parsed->ints[1], parsed->ints[2], parsed->ints[3]);
	return true;
}
COMMAND(glOrtho, "dddddd") {
	glOrtho(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2], parsed->doubles[3], parsed->doubles[4], parsed->doubles[5]);
	return true;
}
COMMAND(glFrustum, "dddddd") {
	glFrustum(parsed->doubles[0], parsed->doubles[1], parsed->doubles[2],
	          parsed->doubles[3], parsed->doubles[4], parsed->doubles[5]);
	return true;
}

/*----------------------------------------------------------------------------
=head2 Display List Functions

=over

=item glNewList LISTNAME MODE

Begin recording a new display list, either creating or overwriting LISTNAME.
MODE can either be GL_COMPILE or GL_COMPILE_AND_EXECUTE.  LISTNAME can be any
string of text and is not limited to OpenGL's integer "names".

=item glEndList

End the recording.

=item glCallList LISTNAME

Replay a recorded list.

=back
*/

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

/*----------------------------------------------------------------------------
=head2 Glu Functions

=over

=item gluLookAt

=item gluNewQuadric NAME

Quadric objects must be created before they can be used.

=item gluQuadricDrawStyle NAME MODE

=item gluQuadricNormals NAME MODE

=item gluQuadricOrientation NAME ORIENTATION

=item gluQuadricTexture NAME MODE

=item gluCylinder

=item gluSphere

=item gluDisk

=item gluPartialDisk

=back
*/

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
