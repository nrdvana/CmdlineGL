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
	glNewList frame GL_COMPILE
	glDisable GL_TEXTURE_2D
	glColor 0.5 0.5 0.5
	DrawFrame 20 20 1
	glEndList

	glNewList cursor GL_COMPILE
	glDisable GL_TEXTURE_2D
	glColor 1 1 1
	glBegin GL_QUADS
	glNormal 0 0 1;
	glVertex 0 0 0;
	glVertex 1 0 0;
	glVertex 1 1 0;
	glVertex 0 1 0;
	glEnd
	glEndList
}

SetLights() {
	glLoadIdentity
	glEnable GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT 0.8 0.8 0.8 0
 	glLight GL_LIGHT0 GL_DIFFUSE 1 0.8 0.8 0
	glLight GL_LIGHT0 GL_SPECULAR 0.8 0.8 0.8 0
	glLight GL_LIGHT0 GL_POSITION -10 10 0 1
}

LoadTextures() {
	cglNewFont CGL_BMPFONT MyFont Arial.bmp
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
	distance=13;
	pitch=0;

	declare -a Lines;
	EditRow=0
	EditCol=0
	CurCol=0
	WindowStart=0
	WindowHeight=12
	CursorBlinkPeriod=250
}

Swap() {
	glFlush
	cglSwapBuffers
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

# Load a file
# @param $1 - Name of file to load
LoadFile() {
	local FName="$1" i=0
	while read "Lines[i]"; do let i++; done < "$FName"
}

# Save to file
# @param $1 - Name of file to write
SaveFile() {
	local FName="$1" LineCnt=${#Lines[@]}
	for ((i=0; i<LineCnt; i++)); do
		echo "${Lines[i]}"
	done > "$Fname"
}

SeekHome() {
	((EditCol=0, CurCol=0));
}

SeekEnd() {
	let EditCol=${#Lines[EditRow]} CurCol=EditCol
}

# Seek horizontally on a line
# Positive numbers indicate east, negative west
# @param $1 - direction/magnitude, as an integer
#
SeekHoriz() {
	local Ofs=$1 LineLen=${#Lines[EditRow]}
	if ((EditCol!=CurCol)); then ((EditCol=CurCol)); fi
	((EditCol+=Ofs))
	if ((EditCol>LineLen)); then
		SeekVert 1
		SeekHome
	elif ((EditCol<0)); then
		if ((EditRow>0)); then
			SeekVert -1
			SeekEnd
		else
			((EditCol= 0))
		fi
	fi
	((CurCol=EditCol))
}

# Seek vertically on the document
# Positive numbers indicate south, negative north.
# @param $1 - direction/magnitude, as an integer
#
SeekVert() {
	local Ofs=$1 LineCnt=${#Lines[@]}
	((EditRow+=Ofs))
	if ((EditRow<0)); then ((EditRow= 0)); fi
	local LineLen=${#Lines[EditRow]}
	if ((EditCol>LineLen)); then ((CurCol=LineLen)); fi
	# Adjust text window
	if ((EditRow>=WindowStart+WindowHeight-1)); then ((WindowStart=EditRow-WindowHeight+1)); fi
	if ((EditRow<=WindowStart+1 && WindowStart>0 && EditRow>0)); then ((WindowStart=EditRow-1)); fi
}

# force the existance of empty row variables
FillEmptyLines() {
	local LineCnt=${#Lines[@]} i
	if ((EditRow>LineCnt)); then
		for ((i=EditRow-1; i>=LineCnt; i--)); do
			Lines[i]="${Lines[i]}"
		done
	fi
}

# Insert text at the current edit position
# @param $@ - text to insert
#
InsertText() {
	FillEmptyLines
	Lines[EditRow]="${Lines[EditRow]:0:EditCol}$@${Lines[EditRow]:EditCol}"
	SeekHoriz ${#@}
}

# @param $1 - direction: 0=delete, -1=backspace
DelChar() {
	local direc=$1 LineLen=${#Lines[EditRow]}
	if ((direc == -1 && CurCol==0)); then
		if ((EditRow > 0)); then
			SeekVert -1
			SeekEnd
			MergeLines $EditRow
		fi
	elif ((direc == 0 && CurCol>=LineLen)); then
		MergeLines $EditRow
	else
		Lines[EditRow]="${Lines[EditRow]:0:CurCol+direc}${Lines[EditRow]:CurCol+direc+1}"
		SeekHoriz $direc
	fi
}

# @param $1 - line number to merge, by appending next line
#
MergeLines() {
	local LineCnt=${#Lines[@]} Line=$1 i
	Lines[Line]="${Lines[Line]}${Lines[Line+1]}"
	for ((i=Line+1; i<LineCnt-1; i++)); do
		Lines[i]="${Lines[i+1]}"
	done
	unset Lines[$LineCnt-1]
}

SplitLine() {
	local LineCnt=${#Lines[@]} NewLine="${Lines[EditRow]:EditCol}" i
	FillEmptyLines
	Lines[EditRow]="${Lines[EditRow]:0:EditCol}"
	for ((i=LineCnt-1; i>EditRow; i--)); do
		Lines[i+1]="${Lines[i]}"
	done
	Lines[EditRow+1]="$NewLine";
	SeekHome
	SeekVert 1
}

DrawStatus() {
	glScale .4 .5 0
	glColor '#00FF00'
	glDisable GL_LIGHTING
	cglText MyFont \""${#Lines[@]}_lines"
	glTranslate 9 0 0;
	cglText MyFont \""($CurCol,$EditRow)"
	glTranslate 9 0 0;
	cglText MyFont \""${FName}"
	glEnable GL_LIGHTING
}

DrawIt() {
	glLoadIdentity
	glTranslate 0 0 -$distance
	glRotate $pitch 1 0 0
	glRotate $direction 0 1 0
	glPushMatrix
	glCallList frame
	glPopMatrix
	glPushMatrix
	glTranslate -9 9.5 0
	DrawStatus
	glPopMatrix
	glColor '#FFFFFF'
	glTranslate -9 7 0
	glPushMatrix
	local Max=${#Lines[@]}
	if ((Max>WindowStart+WindowHeight)); then ((Max=WindowStart+WindowHeight)); fi
	for ((i=WindowStart; i<Max; i++)); do
		cglText MyFont "\"${Lines[i]}"
		glTranslate 0 -1.5 0
	done
	glPopMatrix
	if (( ((Timing_T/CursorBlinkPeriod)&1) == 0 )); then
		glDisable GL_LIGHTING
		cglFixedPt 10
		glTranslate $((CurCol*10)) -$(( (EditRow-WindowStart)*15)) 0.1
		cglFixedPt 1
		glCallList cursor
		glEnable GL_LIGHTING
	fi
	Swap;
}

MouseClick() {
	local Press btn=$2
	if [[ "$1" = "+" ]]; then Press=1; else Press=0; fi
	case "$btn" in
	1) ((LDown=Press));;
	2) ((MDown=Press));;
	3) ((RDown=Press));;
	esac
}

# Handle mouse drag actions.
# If the mouse has moved since last time change the pitch or direction
# by the vertical or horizontal distance the mouse has moved.
# Also repaint the robot and record the "last position" of the mouse.
#
MouseMotion() {
	local dx=$3 dy=$4;
	if ((LDown && RDown)); then
		((distance+= dy))
	else if ((LDown || RDown)); then
		((pitch+= dy, direction+= dx));
	fi; fi
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
		if [[ "$2" = "+" ]]; then
			case "$3$4" in
			right)	SeekHoriz 1;;
			left)	SeekHoriz -1;;
			up)	SeekVert -1;;
			down)	SeekVert 1;;
			enter|return) SplitLine;;
			home)	SeekHome;;
			end)	SeekEnd;;
			backspace) DelChar -1;;
			delete) DelChar 0;;
			?)	InsertText "$3";;
			space)  InsertText " ";;
			escape) terminate=1;;
			esac
			Press=1;
		else
			Press=0;
		fi
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
	if [[ -n "$1" ]]; then
		if [[ -f "$1" ]]; then
			LoadFile "$1"
			FName="$1"
		else
			echo "File not found (or readable): $1" >&2
		fi
	fi

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
	sleep 0.1
}

main "$1" <$CMDLINEGL_PIPE | CmdlineGL >$CMDLINEGL_PIPE
