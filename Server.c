#include "Global.h"
#include "Server.h"

void StartServer(char* Socketname) {
}

void RunCommand(void *Data) {
	int i, code= * (int*) Data;
	char* AsChar= ((char*) Data)+4;
	char** AsCharPtr= ((char**) Data)+1;
	int* AsInt= ((int*) Data)+1;
	double* AsDouble= (double*)(((char*)Data)+4);
	
	switch (code) {
	GLUT_INIT:
		for (i=0; i<AsInt[0]; i++)
			AsInt[1+i]+= (int) AsInt;
		glutInit(AsInt[0], AsCharPtr[1]);
		break;
	GLUT_INITDISPLAYMODE: glutInitDisplayMode(AsInt[0]); break;
	GLUT_CREATEWINDOW:    glutCreateWindow(AsChar); break;
	GLUT_INITWINDOWSIZE:  glutInitWindowSize(AsInt[0], AsInt[1]); break;
	
	GL_BEGIN:        glBegin(AsInt[0]); break;
	GL_END:          glEnd(); break;
	GL_VERTEX2I:     glVertex2iv(AsInt); break;
	GL_VERTEX2:      glVertex2dv(AsDouble); break;
	GL_VERTEX3:      glVertex3dv(AsDouble); break;
	GL_VERTEX4:      glVertex4dv(AsDouble); break;
	GL_NORMAL:       glNormal3dv(AsDouble); break;
	GL_COLOR3:       glColor3dv(AsDouble); break;
	GL_COLOR4:       glColor4dv(AsDouble); break;
	GL_COLOR3b:      glColor3bv(AsChar); break;
	GL_COLOR4b:      glColor4bv(AsChar); break;

	GL_TEXCOORD1:    glTexCoord1dv(AsDouble); break;
	GL_TEXCOORD2:    glTexCoord2dv(AsDouble); break;
	GL_TEXCOORD3:    glTexCoord3dv(AsDouble); break;
	GL_TEXCOORD4:    glTexCoord4dv(AsDouble); break;

	GL_LIGHT:        glLighti(AsInt[0], AsInt[1], (float) AsDouble[1]); break;
	GL_LIGHTMODEL:   glLightModelf(AsInt[0], * (double*) &AsInt[1]); break;
	GL_MATERIAL:     glMaterialf(AsInt[0], AsInt[1], (float) AsDouble[1]); break;
	GL_COLORMATERIAL:glColorMaterial(AsInt[0], AsInt[1]); break;

	GL_BITMAP:       glBitmap(AsInt[0], AsInt[1],
							AsDouble[1], AsDouble[2], AsDouble[3], AsDouble[4]
							(void*) &AsDouble[5]); break;

	GL_MATRIXMODE:   glMatrixMode(AsInt[0]); break;
	GL_ORTHO:        glOrtho(AsDouble[0], AsDouble[1], AsDouble[2], AsDouble[3], AsDouble[4], AsDouble[5]); break;
	GL_FRUSTUM:      glFrustum(AsDouble[0], AsDouble[1], AsDouble[2], AsDouble[3], AsDouble[4], AsDouble[5]); break;
	GL_VIEWPORT:     glViewport(AsInt[0], AsInt[1], AsInt[2], AsInt[3]); break;
	GL_PUSHMATRIX:   glPushMatrix(); break;
	GL_POPMATRIX:    glPopMatrix(); break;
	GL_LOADIDENTITY: glLoadIdentity(); break;
	GL_ROTATE:       glRotated(AsDouble[0], AsDouble[1], AsDouble[2], AsDouble[3]); break;
	GL_SCALE:        glScaled(AsDouble[0], AsDouble[1], AsDouble[2]); break;
	GL_TRANSLATE:    glTranslatef(AsDouble[0], AsDouble[1], AsDouble[2]); break;
	
	GL_NEWLIST:      glNewList(AsInt[0], AsInt[1]); break;
	GL_ENDLIST:      glEndList(); break;
	GL_CALLLIST:     glCallList(AsInt[0]); break;
	}
}


