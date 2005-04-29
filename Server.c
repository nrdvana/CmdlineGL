#include "Global.h"
#include "Server.h"

int Shutdown= 0;

PUBLISHED(quit,DoQuit);
PUBLISHED(exit,DoQuit) {
	Shutdown= true;
}

	// build the address
	Addr.sun_family= AF_UNIX;
//	if (strlen(SocketName) >= sizeof(Addr.sun_path))
//		Err_InvalidSocket();
	strcpy(Addr.sun_path, SocketName);

	// try connecting
	ServerConn= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ServerConn < 0) {
		perror("Socket: ");
//		Err_InvalidSocket();
	}

	if (bind(ServerConn, (struct sockaddr*) &Addr, sizeof(Addr)))
		perror("Bind: ");


//	if (argc > 0)
//		SendCommand(argc, argv);

	while (!Shutdown) {
		red= recv(ServerConn, Buffer, CMD_LEN_MAX, 0);
		write(1, Buffer, red);
//		while (ReadCommand(&argc, NewArgs))
//			InvokeCommand(argc, NewArgs);
		if (strncmp(Buffer, "quit", 4) == 0)
			Shutdown= 1;
	}

	close(ServerConn);
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


