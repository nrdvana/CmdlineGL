#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <ctype.h>
#include "Global.h"
#include "Client.h"
#include "SymbolHash.h"

#define CMD_LEN_MAX 256
#define ARG_COUNT_MAX 8
#define READ_BUFFER_SIZE 1024

bool ReadCommand(int *ArgCountResult, char **ArgResult);
void ProcessCommand(int ArgCount, char** Args);
void SendCommand();
char* Readline();

int ServerConn;
struct sockaddr_un ServerAddr;
char StandardCmdBuffer[64];
void *CmdData;
int CmdDataLen;
char LineBuffer[CMD_LEN_MAX];

void ExecClient(char *SocketName, int argc, char **argv) {
	int CmdArgCount;
	char *CmdArgs[ARG_COUNT_MAX];

	// build the address
	Addr.sun_family= AF_UNIX;
	if (strlen(SocketName) >= sizeof(Addr.sun_path))
		Err_InvalidSocket();
	strcpy(SocketName, Addr.sun_path);

	// try connecting
	ServerConn= socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ServerConn < 0)
		Err_InvalidSocket();

	if (argc > 0)
		ProcessCommand(argc, argv);

	while (ReadCommand(&CmdArgCount, CmdArgs))
		ProcessCommand(CmdArgCount, CmdArgs);

	close(ServerConn);
}

void ProcessCommand(int ArgCount, char** Args) {
	struct CmdHashEntry *Cmd;

	if (ArgCount > 0) {
		Cmd= GetCmd(Args[0]);
		if (Cmd != NULL) {
			CmdData= StandardCmdbuffer;
			Cmd->Value(ArgCount-1, Args+1); // encode command
			SendCommand(Data);
			if (CmdData != StandardCmdBuffer)
				free(CmdData);
		}
		else
			Err_BadCommand();
	}
	else
		Err_BadCommand();
}

void SendCommand(void *Data) {
	sendto(ServerConn, CmdData, CmdDataLen, (struct sockaddr*) ServerAddr, sizeof(ServerAddr));
}

bool ReadCommand(int *ArgCountResult, char **ArgResult) {
	char *LastArg, *temp, *Line;

	Line= Readline();
	if (!Line) return false;

	*ArgCountResult= 0;
	LastArg= NULL;
	temp= strtok(Line, " \t");
	while (temp != NULL && *ArgCountResult < ARG_COUNT_MAX) {
		if (temp > LastArg + 1)
			ArgResult[(*ArgCountResult)++]= temp;
		LastArg= temp;
		temp= strtok(NULL, " \t");
	}
	return true;
}

#define AsChar(i) ((((char*)CmdData)+4)[i])
#define AsDoub(i) (((double*)(((char*)CmdData)+4))[i])
#define AsCharPtr(i) ((((char**) CmdData)+1)[i])
#define AsInt(i) ((((int*)CmdData)+1)[i])

	
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


void EncodeGlutInit(int argc, char**argv) {
	glutInit(argc, argv);
}

void EncodeGlutInitDisplayMode(int argc, char**argv) {
	int param1;
	if (argc == 1 && GetIntParam(&param1,argv[0])) {
		strtol(argv[0]);
		glutInitDisplayMode(
	}
	else
		Err_InvalidParams();
}
void EncodeGlutCreateWindow(int argc, char**argv) {
}
void EncodeGlutInitWindowSize(int argc, char**argv) {
}

void EncodeGlBegin(int argc, char**argv) {
}
void EncodeGlEnd(int argc, char**argv) {
}
void EncodeGlVertex3(int argc, char**argv) {
}

void EncodeGlMatrixMode(int argc, char**argv) {
}
void EncodeGlLoadIdentity(int argc, char**argv) {
}
void EncodeGlPushMatrix(int argc, char**argv) {
}
void EncodeGlPopMatrix(int argc, char**argv) {
}
void EncodeGlScale(int argc, char**argv) {
}
void EncodeGlTranslate(int argc, char**argv) {
}

void EncodeGlViewport(int argc, char**argv) {
}
void EncodeGlOrtho(int argc, char**argv) {
}
void EncodeGlFrustum(int argc, char**argv) {
}

void EncodeGlEnable(int argc, char**argv) {
}
void EncodeGlDisable(int argc, char**argv) {
}

void EncodeGlLight(int argc, char**argv) {
}

void EncodeGlutSwapBuffers(int argc, char**argv) {
}

void EncodeGluCylinder(int argc, char**argv) {
}
void EncodeGluSphere(int argc, char**argv) {
}

bool ParseInt(char* Text, int *Result) {
	struct IntConstHashEntry *SymConst;
	char *EndPtr;
	
	if (Text[0] == 'G' && Text[1] == 'L' && Text[2] != '\0') {
		SymConst= GetIntConst(char *Key);
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

bool ParseDouble(char* Text, double *Result) {
	char *EndPtr;
	*Result= strtod(Text, &EndPtr, 0);
	return (EndPtr != Text);
}

char ReadBuffer[READ_BUFFER_SIZE];
char const *StopPos= ReadBuffer+READ_BUFFER_SIZE-1;
char *Pos= ReadBuffer;
char *DataPos= ReadBuffer;

char* Readline() {
	int shift, red;
	char* LineStart= Pos;
	do {
		if (Pos == DataPos) {
			red= read(0, DataPos, StopPos-DataPos);
			if (red <= 0)
				break;
			DataPos+= red;
		}
		*DataPos= '\0';
		Pos= strchr(Pos, '\n');
		if (Pos == StopPos && LineStart != ReadBuffer) {
			memmove(ReadBuffer, LineStart, Pos-LineStart);
			shift= LineStart - ReadBuffer;
			Pos-= shift;
			DataPos-= shift;
			LineStart-= shift;
		}
	} while (*Pos != '\n' && Pos < StopPos);
	
	if (Pos == LineStart)
		return NULL;
	if (Pos == StopPos) {
		Err_LineTooLong();
		return NULL;
	}
	*Pos= '\0';
	return LineStart;
}



