#! /bin/bash
[ -n "$BASH_VERSION" ] || exec bash $0
set -u
# Define our handy die function
die() { echo "$@" >&2; exit 2; }

source "${BASH_SOURCE%/*}/../share/CmdlineGL.lib" || die "Can't load CmdlineGL.lib  ('${BASH_SOURCE%/*}/../share/CmdlineGL.lib')";

CmdlineGL_LoadLib RenderLoop ModelViewer

Init() {
	glEnable GL_NORMALIZE GL_DEPTH_TEST GL_CULL_FACE
	glEnable GL_LIGHTING
	#glEnable GL_TEXTURE_2D
	glShadeModel GL_SMOOTH
	glClearColor '#000033'

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
	glColor 0.5 0.5 0.5 1
	$Model
	glFlush
	cglSwapBuffers
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

RenderLoop_DispatchEvent() {
	if ! ModelViewer_DispatchEvent "$@"; then
		if [[ "$1" == "K" && "$2" == "+" && "$3" == "q" ]]; then
			RenderLoop_Done=1;
		fi
	fi
}

if (( $# != 1 )); then
	echo 'Usage: ModelViewer.sh LIBNAME'
	echo
	echo ' where LIBNAME is the base-name of a library that defines a renderable model,'
	echo ' such as "Cube" (bash-lib/Cube.lib)'
	echo
elif ! CmdlineGL_LoadLib "$1"; then
	echo 'Failed to load model "$1"'
elif ! [[ "$( type -t "$1" )" == function ]]; then
	echo 'Library "$1.lib" does not define a function named "$1"'
	echo 'This is required'
else
	Model=$1
	CmdlineGL_Start rw || die "Can't init CmdlineGL"
	Init
	RenderLoop_Run
	cglQuit
fi
