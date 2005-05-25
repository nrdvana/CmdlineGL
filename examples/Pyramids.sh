#!/bin/sh
if [ -z "$BASH" ]; then
	exec bash $0
fi

die() { echo $1; exit -1; }

if [ -f ../bin/CmdlineGL_BashBindings ]; then
	source ../bin/CmdlineGL_BashBindings
else
	die "Please 'make' bin/CmdlineGL_BashBindings, and try again"
fi
export CMDLINEGL_PIPE='/tmp/foo';

#export PATH=$PATH:bin/;

#bin/CmdlineGL -f "$CMDLINEGL_PIPE" &
#sleep 1

let x=0;

ToInt() {
	echo -n "${1/./}"
}
ToFloat() {
	let dec_pos=${#1}-$2;
	echo -n ${1:0:$dec_pos}.${1:$dec_pos};
}

Triangle() {
	glBegin GL_TRIANGLES
	glVertex 3 -3 0
	glVertex -3 -3 0
	glVertex 0 3 0
	glEnd
}

Pyramid() {
	glBegin GL_TRIANGLES
	glColor 0.5 0.5 0.2
	glNormal 0 2 4.4
	glVertex 1 -1 1
	glVertex -1 -1 1
	glVertex 0 1.2 0

	glColor 0.5 0.2 0.5
	glNormal -4.4 2 0
	glVertex -1 -1 1
	glVertex -1 -1 -1
	glVertex 0 1.2 0

	glColor 0.2 0.5 0.5
	glNormal 0 2 -4.4
	glVertex -1 -1 -1
	glVertex 1 -1 -1
	glVertex 0 1.2 0

	glColor 0.2 0.7 0.5
	glNormal 4.4 2 0
	glVertex 1 -1 -1
	glVertex 1 -1 1
	glVertex 0 1.2 0
	glEnd

	glColor 0.2 0.2 0.5
	glBegin GL_QUADS
	glNormal 0 -1 0
	glVertex 1 -1 -1
	glVertex -1 -1 -1
	glVertex -1 -1 1
	glVertex 1 -1 1
	glEnd
}

BuildList() {
	glLoadIdentity
	glNewList tri GL_COMPILE
	glColor 0.5 0.5 0.5
	Pyramid
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
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	glClear GL_COLOR_BUFFER_BIT
	glClear GL_DEPTH_BUFFER_BIT
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
	
	glTranslate 3 0 0
	glCallList tri
	glTranslate -3 0 3
	glCallList tri
	glTranslate -3 0 -3
	glCallList tri
	glTranslate 3 0 -3
	glCallList tri
	glTranslate 0 0 3
	glRotate $x 0 0 1
	glRotate $x 0 1 1
	Pyramid
	cglSync $frametime
	Swap;
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

[ -e "$CMDLINEGL_PIPE" ] && { rm -f "$CMDLINEGL_PIPE" || die "Cannot remove $CMDLINEGL_PIPE"; }
[ -e "/tmp/bar" ] && { rm -f "/tmp/bar" || die "Cannot remove /tmp/bar"; }

mkfifo /tmp/bar

(
	cglEcho "_____";
	Init
	BuildList
	SetLights
	Swap
	frametime=0;
	direction=0;
	distance=10;
	terminate='';
	pitch=0;
	while [[ -z "$terminate" ]]; do
		ReadInput;
		cglEcho "_____";
		ProcessInput
		DrawIt
		let x++;
		if [ $x -eq 360 ]; then x=0; fi
		let frametime=$frametime+33;
	done
	cglQuit
) < /tmp/bar | ../bin/CmdlineGL > /tmp/bar

