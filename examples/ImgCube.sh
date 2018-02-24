#! /bin/bash
[ -n "$BASH_VERSION" ] || exec bash $0
set -vue
# Define our handy die function
die() { echo "$@" >&2; exit 2; }

source "${BASH_SOURCE%/*}/../share/CmdlineGL.lib" || die "Can't find ../share directory (from $PWD via ${BASH_SOURCE%/*})";

CmdlineGL_LoadLib RenderLoop ModelViewer

Face() {
	glBegin GL_QUADS
	glNormal 0 0 1;
	glTexCoord 0 1; glVertex -1 -1 1;
	glTexCoord 1 1; glVertex 1 -1 1;
	glTexCoord 1 0; glVertex 1 1 1;
	glTexCoord 0 0; glVertex -1 1 1;
	glEnd
}

Cube() {
	glNewList cube GL_COMPILE
	Face
	glRotate 90 0 1 0
	Face
	glRotate 90 0 1 0
	Face
	glRotate 90 0 1 0
	Face
	glRotate 90 1 0 0
	Face
	glRotate 180 1 0 0
	Face
	glEndList
	# Optimize future calls
	Cube() { glCallList cube; }
}

Init() {
	glEnable GL_NORMALIZE GL_DEPTH_TEST GL_CULL_FACE
	glEnable GL_LIGHTING
	glEnable GL_TEXTURE_2D
	glShadeModel GL_SMOOTH
	glClearColor '#000033'
	#glEnable GL_COLOR_MATERIAL

	CmdlineGL_LoadTex checker

	# Lighting
	glLoadIdentity
	glEnable GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT 0.8 0.8 0.8 0
 	glLight GL_LIGHT0 GL_DIFFUSE 1 0.8 0.8 0
	glLight GL_LIGHT0 GL_SPECULAR 0.8 0.8 0.8 0
	glLight GL_LIGHT0 GL_POSITION 10 10 10 1

	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

RenderLoop_Render() {
	ModelViewer_Update
	
	glLoadIdentity
	ModelViewer_ApplyMatrix
	glColor 0.5 0.5 0.5
	glBindTexture GL_TEXTURE_2D checker
	Cube
	glFlush
	cglSwapBuffers
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

RenderLoop_DispatchEvent() {
	ModelViewer_DispatchEvent "$@"
}

main () {
	Init
	RenderLoop_Run
	cglQuit
}

if (( $# < 1 )); then
	CmdlineGL_Start rw || die "Can't init CmdlineGL"
	main
elif [[ "$1" == "--record" ]]; then
	CmdlineGL() { tee replay | command CmdlineGL; }
	CmdlineGL_Start rw || die "Can't init CmdlineGL"
	main
elif [[ "$1" == "--dump" ]]; then
	CmdlineGL_Start stdout || die "Can't init CmdlineGL state"
	main
else
	echo 'Usage: ImgCube.sh [ --record | --dump ]'
	echo
	echo '   --dump    Dump all output to stdout at a virtual 40fps'
	echo '   --record  Run CmdlineGL, but duplicate all output to "./replay"'
	echo
	echo '   Recordings can be played by piping them into CmdlineGL.'
	echo '   For instance:'
	echo '         $ CmdlineGL <replay >/dev/null'
fi
