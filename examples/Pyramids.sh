#! /bin/bash
[ -n "$BASH_VERSION" ] || exec bash $0

# Define our handy die function
die() { echo "$@" >&2; exit 2; }

# Load bash libraries
source "${BASH_SOURCE%/*}/../share/CmdlineGL.lib" RenderLoop Timing \
	|| die "Can't find ../share directory (from $PWD via ${BASH_SOURCE%/*})"

let x=0;

ToInt() {
	Result=${1/./}
}
ToFloat() {
	let dec_pos=${#1}-$2;
	Result=${1:0:dec_pos}.${1:dec_pos};
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
	BuildList
	SetLights

	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT

	frametime=0;
	direction=0;
	distance=10;
	pitch=0;
}

Swap() {
	glFlush
	cglSwapBuffers
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

RenderLoop_Render() {
	Update; # apply current user input to program state
	
	ToFloat 000$((Timing_T*8%36000)) 2
	x=$Result
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
	glCallList tri
	Swap;
}

MouseClick() {
	local Press btn=$2
	if [[ "$1" = "+" ]]; then Press=1; else Press=0; fi
	if ((btn==1)); then
		((Dragging=Press))
	fi
}

# Handle mouse drag actions.
# If the mouse has moved since last time change the pitch or direction
# by the vertical or horizontal distance the mouse has moved.
# Also repaint the robot and record the "last position" of the mouse.
#
MouseMotion() {
	local dx=$3 dy=$4;
	if ((Dragging)); then
		if ((dy)); then
			((pitch+= dy))
		fi
		if ((dx)); then
			((direction+= dx));
		fi
	fi
}

RenderLoop_DispatchEvent() {
	local Press
	case "$1" in
	K)
		if [[ "$2" = "+" ]]; then Press=1; else Press=0; fi
		case "$3" in
		right)  PanRight=$Press;;
		left)   PanLeft=$Press;;
		up)     PanUp=$Press;;
		down)   PanDn=$Press;;
		=)      ZoomIn=$Press;;
		-)      ZoomOut=$Press;;
		q)      RenderLoop_Done=1;;
		esac
		;;
	M)
		if [[ "$2" = "@" ]]; then
			MouseMotion $3 $4 $5 $6
		else
			MouseClick $2 $3
		fi
		;;
	esac
}

Update() {
	if ((PanLeft)); then let direction+=2; fi
	if ((PanRight));then let direction-=2; fi
	if ((PanUp));   then let pitch-=2; fi
	if ((PanDn));   then let pitch+=2; fi
	if ((ZoomIn));  then let distance-=1; fi
	if ((ZoomOut)); then let distance+=1; fi
}

main() {
	Init
	Swap
	RenderLoop_Run;
	cglQuit
}

if [[ "$1" == "--record" ]]; then
	CmdlineGL() { tee replay | command CmdlineGL; }
	CmdlineGL_Start rw || die "Can't init CmdlineGL"
	main
elif [[ "$1" == "--dump" ]]; then
	CmdlineGL_Start stdout || die "Can't init CmdlineGL state"
	main
elif [[ -z "$1" ]]; then
	CmdlineGL_Start rw || die "Can't init CmdlineGL"
	main
else
	echo 'Usage: Pyramids.sh [ --record | --dump ]'
	echo
	echo '   --dump    Dump all output to stdout at a virtual 40fps'
	echo '   --record  Run CmdlineGL, but duplicate all output to "./replay"'
	echo
	echo '   Recordings can be played by piping them into CmdlineGL.'
	echo '   For instance:'
	echo '         $ CmdlineGL <replay >/dev/null'
fi
