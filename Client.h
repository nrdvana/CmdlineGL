#ifndef CLIENT_H
#define CLIENT_H

void ExecClient(char *socket, int argc, char **argv);

#define DATA // HashTable data:
void EncodeGlutInit(int argc, char**argv);
DATA utInit EncodeGlutInit
void EncodeGlutInitDisplayMode(int argc, char**argv);
DATA utInitDisplayMode EncodeGlutInitDisplayMode
void EncodeGlutCreateWindow(int argc, char**argv);
DATA utCreateWindow EncodeGlutCreateWindow
void EncodeGlutInitWindowSize(int argc, char**argv);
DATA utInitWindowSize EncodeGlutInitWindowSize

void EncodeGlBegin(int argc, char**argv);
DATA Begin EncodeGlBegin
void EncodeGlEnd(int argc, char**argv);
DATA End EncodeGlEnd
void EncodeGlVertex3(int argc, char**argv);
DATA Vertex3 EncodeGlVertex3

void EncodeGlMatrixMode(int argc, char**argv);
DATA MatrixMode EncodeGlMatrixMode
void EncodeGlLoadIdentity(int argc, char**argv);
DATA LoadIdentity EncodeGlLoadIdentity
void EncodeGlPushMatrix(int argc, char**argv);
DATA Pushmatrix EncodeGlPushMatrix
void EncodeGlPopMatrix(int argc, char**argv);
DATA PopMatrix EncodeGlPopMatrix
void EncodeGlScale(int argc, char**argv);
DATA Scale EncodeGlScale
void EncodeGlTranslate(int argc, char**argv);
DATA Translate EncodeGlTranslate

void EncodeGlViewport(int argc, char**argv);
DATA Viewport EncodeGlViewport
void EncodeGlOrtho(int argc, char**argv);
DATA Ortho EncodeGlOrtho
void EncodeGlFrustum(int argc, char**argv);
DATA Frustum EncodeGlFrustum

void EncodeGlEnable(int argc, char**argv);
DATA Enable EncodeGlEnable
void EncodeGlDisable(int argc, char**argv);
DATA Disable EncodeGlDisable

void EncodeGlLight(int argc, char**argv);
DATA Light EncodeGlLight

void EncodeGlutSwapBuffers(int argc, char**argv);
DATA utSwapBuffers EncodeGlutSwapBuffers

void EncodeGluCylinder(int argc, char**argv);
DATA uCylinder EncodeGluCylinder
void EncodeGluSphere(int argc, char**argv);
DATA uSphere EncodeGluSphere
#endif
