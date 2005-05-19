#!/bin/sh

source BashInterpKluge.sh

die() { echo $1; exit -1; }

if [ -f ../bin/CmdlineGL_BashBindings ]; then
	source ../bin/CmdlineGL_BashBindings
else
	die "Please 'make' bin/CmdlineGL_BashBindings, and try again"
fi
export CMDLINEGL_PIPE='/tmp/foo';

BuildList() {
	${ModelName}Init
	glLoadIdentity
	glColor 0.5 0.5 0.5
	glNewList model GL_COMPILE
	$ModelName
	glEndList
}

SetLights() {
	glLoadIdentity
	glEnable GL_LIGHTING
	glEnable GL_COLOR_MATERIAL
	glEnable GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT 0.8 0.8 0.8 0
 	glLight GL_LIGHT0 GL_DIFFUSE 1 0.8 0.8 0
	glLight GL_LIGHT0 GL_SPECULAR 0.8 0.8 0.8 0
	glLight GL_LIGHT0 GL_POSITION 10 10 10 1
}

Init() {
	cglEcho "_____";
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	glEnable GL_TEXTURE_2D
	glEnable GL_CULL_FACE
	glShadeModel GL_SMOOTH
	glClear GL_COLOR_BUFFER_BIT
	glClear GL_DEPTH_BUFFER_BIT
	glBindTexture GL_TEXTURE_2D bmp
	cglLoadImage2D /tmp/temp.bmp
	glTexParameter GL_TEXTURE_2D GL_TEXTURE_MIN_FILTER GL_LINEAR
	glTexParameter GL_TEXTURE_2D GL_TEXTURE_MAG_FILTER GL_LINEAR
	SetLights
	BuildList
}

Swap() {
	glFlush
	glutSwapBuffers
	glClear GL_COLOR_BUFFER_BIT
	glClear GL_DEPTH_BUFFER_BIT
}

DrawIt() {
	glLoadIdentity
	glTranslate 0 0 -$distance
	glRotate $pitch 1 0 0
	glRotate $direction 0 1 0

	glBegin GL_LINES
	glColor 1 0 0
	glVertex 1 0 0
	glVertex 0 0 0
	glColor 0 1 0
	glVertex 0 1 0
	glVertex 0 0 0
	glColor 0 0 1
	glVertex 0 0 1
	glVertex 0 0 0
	glEnd
	
	glCallList model
}

ReadInput() {
	while read -r -t 1 Action; do
		case "$Action" in
		+LEFT)  K_Left=true;;
		-LEFT)  K_Left='';;
		+RIGHT) K_Right=true;;
		-RIGHT) K_Right='';;
		+UP)	K_Up=true;;
		-UP)    K_Up='';;
		+DOWN)  K_Down=true;;
		-DOWN)  K_Down='';;		
		+=)     K_In=true;;
		-=)     K_In='';;
		+-)     K_Out=true;;
		--)     K_Out='';;
		+q)     terminate=1;;
		"_____")
			return;
			;;
		-*)
			;;
		*)
			echo "Unhandled IO: $Action" >&2
			;;
		esac
	done
}

ProcessInput() {
	if [ -n "$K_Left"  ]; then let direction+=2; fi
	if [ -n "$K_Right" ]; then let direction-=2; fi
	if [ -n "$K_Up"    ]; then let pitch-=2; fi
	if [ -n "$K_Down"  ]; then let pitch+=2; fi
	if [ -n "$K_In"    ]; then let distance--; fi
	if [ -n "$K_Out"   ]; then let distance++; fi
}

main() {
	[[ -z "$NonInteractive" ]] && cglEcho "_____";

	# Initialize our own stuff (and OpenGL lighting).
	Init;

	Swap
	frametime=0;
	direction=0;
	distance=10;
	terminate='';
	pitch=0;
	# repaint forever
	while [[ -z "$terminate" ]]; do
		if [[ -z "$NonInteractive" ]]; then
			ReadInput;
			cglEcho "_____";
		fi
		ProcessInput;
		DrawIt
		cglSync $frametime
		Swap;
		let frametime=$frametime+33;
	done
	cglQuit
}

if [[ "$1" = "--dump" ]]; then
	NonInteractive=true;
	shift
fi

ModelName=$1
if [[ ! -f $ModelName.model ]]; then
	die "$1.model not found."
fi

source $ModelName.model;

if [[ -n "$NonInteractive" ]]; then
	main
else
	rm -f /tmp/cgl_fifo
	mkfifo /tmp/cgl_fifo
	main < /tmp/cgl_fifo | ../bin/CmdlineGL >> /tmp/cgl_fifo
fi
