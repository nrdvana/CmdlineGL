#! /bin/sh
if [ -z "$BASH" ]; then
	exec bash $0
fi

# Define our handy die function
die() { echo $1; exit -1; }

# Make sure we have CmdlineGL
PATH=../output:$PATH
which CmdlineGL >/dev/null || die "Make sure CmdlineGL is in the PATH"

# Initialize the pipe location, if not already set
if [ -z "$CMDLINEGL_PIPE" ]; then
	CMDLINEGL_PIPE="/tmp/${USER}_CGL/fifo";
fi

# Create the fifo for attaching CmdlineGL to the script
if [ -e "$CMDLINEGL_PIPE" ]; then
	rm -f "$CMDLINEGL_PIPE" || die "Cannot remove $CMDLINEGL_PIPE";
else
	[[ -n "${CMDLINEGL_PIPE%/*}" ]] && mkdir -p "${CMDLINEGL_PIPE%/*}"
fi
mkfifo $CMDLINEGL_PIPE || die "Cannot create $CMDLINEGL_PIPE needed for communication between script and CmdlineGL"

# build functions for each available command
for cmd in `CmdlineGL --showcmds`; do
	eval "$cmd() { echo \"$cmd \$@\"; }"
done

if [[ -f Timing.sh ]]; then
	source Timing.sh
else
	die "Need the timing library, Timing.sh"
fi

# @Param $1 - width
# @Param $2 - height
# @Param $3 - border width
DrawFrame() {
	local w=$1 h=$2 border=$3 Xin Xout Yin Yout Zback
	((Xin=w>>1, Xout=Xin+border, Yin=h>>1, Yout=Xout+border, Zback=-border))
	glBegin GL_QUADS
	glNormal 0 1 1;
	glVertex -$Xout -$Yout 0; glVertex $Xout -$Yout 0;
		glVertex $Xin -$Yin $Zback; glVertex -$Xin -$Yin $Zback; # bottom
	glNormal 0 -1 1;
	glVertex $Xout $Yout 0; glVertex -$Xout $Yout 0;
		glVertex -$Xin $Yin $Zback; glVertex $Xin $Yin $Zback; # top
	glNormal 1 0 1
	glVertex -$Xout $Yout 0; glVertex -$Xout -$Yout 0;
		glVertex -$Xin -$Yin $Zback; glVertex -$Xin $Yin $Zback; # left
	glNormal -1 0 1
	glVertex $Xout -$Yout 0; glVertex $Xout $Yout 0;
		glVertex $Xin $Yin $Zback; glVertex $Xin -$Yin $Zback; # right
	glNormal 0 0 1
	glVertex -$Xin -$Yin $Zback; glVertex $Xin -$Yin $Zback;
		glVertex $Xin $Yin $Zback; glVertex -$Xin $Yin $Zback; # backplane
	glEnd
}

BuildList() {
	glLoadIdentity
	glNewList frame GL_COMPILE
	glDisable GL_TEXTURE_2D
	glColor 0.5 0.5 0.5
	DrawFrame 20 20 1
	glEndList
}

SetLights() {
	glLoadIdentity
	glEnable GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT 0.8 0.8 0.8 0
 	glLight GL_LIGHT0 GL_DIFFUSE 1 0.8 0.8 0
	glLight GL_LIGHT0 GL_SPECULAR 0.8 0.8 0.8 0
	glLight GL_LIGHT0 GL_POSITION -10 10 15 1
}

LoadTextures() {
	cglNewFont CGL_BMPFONT MyFont Font2.bmp
}

Init() {
	glEnable GL_NORMALIZE GL_DEPTH_TEST GL_CULL_FACE
	glEnable GL_LIGHTING
	glEnable GL_TEXTURE_2D
	glShadeModel GL_SMOOTH
	glClearColor '#000033'

	#glEnable GL_COLOR_MATERIAL

	LoadTextures
	BuildList
	SetLights

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

DrawIt() {
	glLoadIdentity
	glTranslate 0 0 -$distance
	glRotate $pitch 1 0 0
	glRotate $direction 0 1 0
#	glScale 1 3 3
#	glCallList frame
	glTranslate -5 -1 0
	cglText MyFont 'ABCDEFjgy1234567890{}'
	glTranslate 0 2 0
	cglText MyFont '0@Oo{}[]()|/\'
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

ReadInput() {
	local ReadMore=1 ReadTimeout=0
	while ((ReadMore)); do
		if read -r Input; then
			if [[ "${Input:0:2}" = "t=" ]]; then
				# The time is the last thing we read for this frame's input
				UpdateTime ${Input:2}
				ReadMore=0
			else
				ProcessInput $Input
			fi
		else
			let ReadTimeout++
			if ((ReadTimeout>3)); then
				terminate=1;
				ReadMore=0;
				die "Reading from pipe timed out...  shutting down."
			fi
		fi
	done
	cglGetTime
}

ProcessInput() {
	local Press
	case "$1" in
	K)
		if [[ "$2" = "+" ]]; then Press=1; else Press=0; fi
		case "$3" in
		right)	PanRight=$Press;;
		left)	PanLeft=$Press;;
		up)		PanUp=$Press;;
		down)	PanDn=$Press;;
		=)		ZoomIn=$Press;;
		-)		ZoomOut=$Press;;
		q)		terminate=1;;
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

main () {
	cglGetTime;
	Init
	LastFPSWrite=$Timing_T
	Frame=0
	Swap

	terminate='';
	while [[ -z "$terminate" ]]; do
		DrawIt
		((Frame++))
		if ((Timing_T-LastFPSWrite >1000)); then
			echo "FPS: $Timing_FPS  $Frame" >&2
			((LastFPSWrite+=1000, Frame=0))
		fi
		SyncNextFrame
		ReadInput
		Update
	done
	cglQuit
}

if [[ "$1" == "--record" ]]; then
	main <$CMDLINEGL_PIPE | tee replay | CmdlineGL >$CMDLINEGL_PIPE
elif [[ "$1" == "--dump" ]]; then
	cglGetTime() { return; }
	ReadInput() { UpdateTime $((Timing_T+25)); }
	main
elif [[ -z "$1" ]]; then
	main <$CMDLINEGL_PIPE | CmdlineGL >$CMDLINEGL_PIPE
else
	echo 'Usage: Pyramids.sh [ --record | --dump ]'
	echo
	echo '   --dump    Dump all output to stdout at a virtual 40fps'
	echo '   --record  Run CmdlineGL, but duplicate all output to "./replay"'
	echo
	echo '   Recordings can be played by piping them into CmdlineGL.'
	echo '   For instance:'
	echo '         $ cat replay | CmdlineGL >/dev/null'
fi

