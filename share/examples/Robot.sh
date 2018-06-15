#! /bin/bash
[ -n "$BASH_VERSION" ] || exec bash $0
set -u
# Define our handy die function
die() { echo "$@" >&2; exit 2; }

source "${BASH_SOURCE%/*}/../CmdlineGL.lib" || die "Can't find CmdlineGL.lib (${BASH_SOURCE%/*}/../CmdlineGL.lib)";
CmdlineGL_LoadLib RenderLoop ModelViewer

#------------------------------------------------------------------------------
# This is an almost line-by-line conversion from C to bash
# The conversion process was rather fun in a strange morbid way...
#
# Actually, the program is somewhat improved, since it uses display lists now
#
#------------------------------------------------------------------------------
# Constants for the dimensions of the robot's body
#
#                   ___
#                  /   \
#   Y              \   /
#   |             -_____-
#   |        -----|     |-----
#   |        |    |     |    |
#   |______X |----|     |----|
#  / 0       | |  |     |  | |
# /           \\  |     |   \\
# Z            \\ -__-__-    \\
#               \\_ | | __    \\
#               |  LJ LJ  |
#               |  r|_r|  |
#               |  |   |  |
#               |  |   |  |
#               |--|   |--|
#               |  |   |  |
#               |  |   |  |
#               |__|   |__|
#              |    | |    |
#              ------ ------
#

HEAD_RADIUS=100;
TORSO_LENGTH=480;
TORSO_RADIUS=90;
HIP_WIDTH=220;
HIP_SPACING=20;
HIP_RADIUS=35;
SHOULDER_WIDTH=500;
SHOULDER_RADIUS=70;
UPPER_ARM_LENGTH=300;
UPPER_ARM_RADIUS=40;
LOWER_ARM_LENGTH=280;
LOWER_ARM_RADIUS=30;
UPPER_LEG_LENGTH=340;
UPPER_LEG_RADIUS=55;
LOWER_LEG_LENGTH=300;
LOWER_LEG_RADIUS=45;
FOOT_LENGTH=160;
FOOT_HEIGHT=30;
HEAD_HEIGHT=$((TORSO_LENGTH+$HEAD_RADIUS));
PELVIC_HEIGHT=$((-HIP_RADIUS*18/10));
SHOULDER_HEIGHT=$((TORSO_LENGTH*84/100));
SHOULDER_OFFSET=$((SHOULDER_WIDTH/2 - $UPPER_ARM_RADIUS));
ELBOW_RADIUS=$((LOWER_ARM_RADIUS*12/10));
ELBOW_WIDTH=$((LOWER_ARM_RADIUS*24/10));
LEG_OFFSET=$((HIP_WIDTH/2 ));
UPPER_LEG_PIPE_LENGTH=$((UPPER_LEG_LENGTH+UPPER_LEG_RADIUS));
HIP_SUPPORT_WIDTH=$((HIP_WIDTH - HIP_SPACING*2 - UPPER_LEG_RADIUS*2));
HIP_SUPPORT_HALF_WIDTH=$((HIP_SUPPORT_WIDTH/2));
KNEE_RADIUS=$((LOWER_LEG_RADIUS*12/10));
KNEE_WIDTH=$((LOWER_LEG_RADIUS*24/10));

# Indicies of important angles
#
NeckX=0; NeckY=1;
LShoulder=2; RShoulder=3; LElbow=4; RElbow=5;
Torso=6;
LHip=7; RHip=8; LKnee=9; RKnee=10; LFoot=11; RFoot=12;

# Joint records for the motionless robot and the four stage animation
#
Standing=(    0  0  1000  1000 -2000 -2000      0       0     0     0     0  1000  1000 );
WalkScript0=( 0  0  2000  2000 -5500 -5500      0   -4000  1000  7000  1000 -2000  -500 );
WalkScript1=( 0  0  4000     0 -4000 -7000   -700   -3000  2500  2000  3000  2000  3000 );
WalkScript2=( 0  0  2000  2000 -5500 -5500      0    1000 -4000  1000  7000  -500 -2000 );
WalkScript3=( 0  0     0  4000 -7000 -4000    700    2500 -3000  3000  2000  3000  2000 );


# Variables that describe the current robot
#
Robot_Joints=0;
Robot_MoveProgress=0;
Robot_Animate=true;

# Variables related to the camera
#
View_Direction=0;
View_Pitch=0;
View_Distance=1000;
View_Mode=0;

Dragging=0;

# Draw the head of the robot.
# Draws a sphere with radius "HEAD_RADIUS" at the current origin.
#
build_head() {
	glNewList head GL_COMPILE
	glPushMatrix
	glScale $HEAD_RADIUS $HEAD_RADIUS $HEAD_RADIUS
	gluSphere quadric 100 10 10
	glPopMatrix
	glEndList
}
head() {
	glCallList head
}

# Draw a closed cylinder.
# This is equivalent to a
#
closedCylinder() { # GLUquadricObj *qobj GLdouble baseRadius GLdouble topRadius GLdouble height GLint slices GLint stacks
	baseRadius=$2; topRadius=$3; height=$4; slices=$5; stacks=$6;
	gluCylinder $1 0 $baseRadius 0 $slices 1
	gluCylinder $1 $baseRadius $topRadius $height $slices $stacks
	glTranslate 0 0 $height
	gluCylinder $1 $topRadius 0 0 $slices 1
	glTranslate 0 0 -$height
}

# Draw the torso of the robot.
# This consists of a large vertical cylinder above the origin a tapered
# cylinder on the bottom of it and horizontal shoulder cylinder.
# The origin ends up located at the very bottom of the torso.
#
build_torso() {
	glNewList torso GL_COMPILE
	glPushMatrix
	glTranslate 0 $TORSO_LENGTH 0
	glRotate 9000 100 0 0
	local TorsoZ=$((TORSO_LENGTH*8/10));
	closedCylinder quadric $TORSO_RADIUS $TORSO_RADIUS $TorsoZ 20 1
	glTranslate 0 0 $TorsoZ
	closedCylinder quadric $TORSO_RADIUS $((HIP_SUPPORT_WIDTH/2)) $((TORSO_LENGTH*2/10)) 20 1
	glPopMatrix

	glPushMatrix
	glTranslate $((-SHOULDER_WIDTH/2)) $SHOULDER_HEIGHT 0.0
	glRotate 9000 0 100 0
	closedCylinder quadric $SHOULDER_RADIUS $SHOULDER_RADIUS $SHOULDER_WIDTH 20 1
	glPopMatrix
	glEndList
}
torso() {
	glCallList torso
}

# Draw the lower torso of the robot.
# This piece is composed of a vertical disc (short horizontal cylinder) and a
# long horizontal cylinder that runs through it.
#
build_lower_torso() {
	glNewList lower_torso GL_COMPILE
	glPushMatrix
	glTranslate 0 $PELVIC_HEIGHT 0
	glRotate 9000 0 100 0
	glTranslate 0 0 -$HIP_SUPPORT_HALF_WIDTH
	closedCylinder quadric $TORSO_RADIUS $TORSO_RADIUS $HIP_SUPPORT_WIDTH 20 1

	glTranslate 0 0 -$(($HIP_SPACING + $UPPER_LEG_RADIUS))
	gluCylinder quadric $HIP_RADIUS $HIP_RADIUS $HIP_WIDTH 10 1
	glPopMatrix
	glEndList
}
lower_torso() {
	glCallList lower_torso
}

# Draw the upper arm of the robot.
# This is a simple cylinder that runs vertically from its origin downward
# toward the elbow.
#
build_upper_arm() {
	glNewList upper_arm GL_COMPILE
	glPushMatrix
	glRotate 9000 100 0 0
	gluCylinder quadric $UPPER_ARM_RADIUS $LOWER_ARM_RADIUS $UPPER_ARM_LENGTH 20 1
	glPopMatrix
	glEndList
}
upper_arm() {
	glCallList upper_arm
}

# Draw the elbow and lower arm.
# This function draws a short horizontal cylinder at the origin then draws
# lower arm extending downward from there.
#
build_lower_arm() {
	glNewList lower_arm GL_COMPILE
	glPushMatrix
	glRotate 9000 0 100 0
	glTranslate 0 0 $((-ELBOW_WIDTH/2))
	closedCylinder quadric $ELBOW_RADIUS $ELBOW_RADIUS $ELBOW_WIDTH 20 1
	glPopMatrix

	glPushMatrix
	glRotate 9000 100 0 0
	closedCylinder quadric $LOWER_ARM_RADIUS $LOWER_ARM_RADIUS $LOWER_ARM_LENGTH 20 1
	glPopMatrix
	glEndList
}
lower_arm() {
	glCallList lower_arm
}

# Draw the upper leg.
# This function draws a simple cylinder that starts just above the origin and
# extends downward.
#
build_upper_leg() {
	glNewList upper_leg GL_COMPILE
	glPushMatrix
	glTranslate 0 $UPPER_LEG_RADIUS 0
	glRotate 9000 100 0 0
	closedCylinder quadric $UPPER_LEG_RADIUS $LOWER_LEG_RADIUS $UPPER_LEG_PIPE_LENGTH 20 1
	glPopMatrix
	glEndList
}
upper_leg() {
	glCallList upper_leg
}

# Draw the knee and lower leg.
# This draws a short horizontal cylinder for the knee then draws a vertical
# cylinder for the lower leg.
#
build_lower_leg() {
	glNewList lower_leg GL_COMPILE
	glPushMatrix
	glRotate 9000 0 100 0
	glTranslate 0 0 $((-KNEE_WIDTH/2))
	closedCylinder quadric $KNEE_RADIUS $KNEE_RADIUS $KNEE_WIDTH 20 1
	glPopMatrix

	glPushMatrix
	glRotate 9000 1 0 0
	gluCylinder quadric $LOWER_LEG_RADIUS $LOWER_LEG_RADIUS $LOWER_LEG_LENGTH 20 1
	glPopMatrix
	glEndList
}
lower_leg() {
	glCallList lower_leg
}

# Draw the ankle and foot.
# This draws a short horizontal cylinder for the ankle/heel and then draws
# a flattened tapered cylinder along the Z axis for the foot.
#
build_foot() {
	glNewList foot GL_COMPILE
	glPushMatrix
	glRotate 9000 0 100 0
	glTranslate 0 0 $((-LOWER_LEG_RADIUS))
	closedCylinder quadric $LOWER_LEG_RADIUS $LOWER_LEG_RADIUS $((LOWER_LEG_RADIUS*2)) 20 1
	glPopMatrix
	glPushMatrix
	glTranslate 0 $((-LOWER_LEG_RADIUS*3/10)) 0
	glScale 100 30 100
	closedCylinder quadric $((LOWER_LEG_RADIUS*8/10)) $((LOWER_LEG_RADIUS*12/10)) $FOOT_LENGTH 20 1
	glPopMatrix
	glEndList
}
foot() {
	glCallList foot
}

# Display the robot using the joint angles in the Robot structure.
#
# This function starts from the identity matrix and first modifies the matrix
# to put the camera in the desired position.
#
# It then traverses the structure of the robot's body calling each of
# the drawing functions in thew process.
#
# Last it swaps the drawing buffers to make the new image visible.
#
Repaint() {
	# Clear the drawing buffer
	glClear GL_COLOR_BUFFER_BIT
	glClear GL_DEPTH_BUFFER_BIT
	glLoadIdentity

	# Alter the model matrix to give the effect of having a movable camera.
	ModelViewer_ApplyMatrix

	glPushMatrix
	# Move to the center of the head rotate by the neck angles and draw.
	glTranslate 0 $HEAD_HEIGHT 0
	glRotate ${Robot_Joints[$NeckX]} 100 0 0
	glRotate ${Robot_Joints[$NeckY]} 0 100 0
		head
	glPopMatrix

	# Draw the torso.  It never needs rotated.
	torso

	glPushMatrix
	# Draw the left arm.  Start at the left shoulder draw the uper arm
	#  then translate down to the elbow rotate and draw the lower arm.
	glTranslate $SHOULDER_OFFSET $SHOULDER_HEIGHT 0
	glRotate ${Robot_Joints[$LShoulder]} 100 0 0
		upper_arm
		glTranslate 0 -$UPPER_ARM_LENGTH 0
		glRotate ${Robot_Joints[$LElbow]} 100 0 0
			lower_arm
	glPopMatrix

	glPushMatrix
	# Same for the right arm.
	glTranslate -$SHOULDER_OFFSET $SHOULDER_HEIGHT 0
	glRotate ${Robot_Joints[$RShoulder]} 100 0 0
		upper_arm
		glTranslate 0 -$UPPER_ARM_LENGTH 0
		glRotate ${Robot_Joints[$RElbow]} 100 0 0
			lower_arm
	glPopMatrix

	glPushMatrix
	# Rotate for the hips draw them then draw the left and right leg.
	glRotate ${Robot_Joints[$Torso]} 0 100 0
		lower_torso

		glPushMatrix
		# First move to the left draw the upper leg and then translate down
		#  to the knee draw the lower leg then translate down to the foot
		#  then rotate and draw it.
		glTranslate $LEG_OFFSET $PELVIC_HEIGHT 0
		glRotate ${Robot_Joints[$LHip]} 100 0 0
			upper_leg
			glTranslate 0 -$UPPER_LEG_LENGTH 0
			glRotate ${Robot_Joints[$LKnee]} 100 0 0
				lower_leg
				glTranslate 0 -$LOWER_LEG_LENGTH 0
				glRotate ${Robot_Joints[$LFoot]} 100 0 0
				foot
		glPopMatrix

		glPushMatrix
		# Same for the right leg.
		glTranslate -$LEG_OFFSET $PELVIC_HEIGHT 0
		glRotate ${Robot_Joints[$RHip]} 100 0 0
			upper_leg
			glTranslate 0 -$UPPER_LEG_LENGTH 0
			glRotate ${Robot_Joints[$RKnee]} 100 0 0
				lower_leg
				glTranslate 0 -$LOWER_LEG_LENGTH 0
				glRotate ${Robot_Joints[$RFoot]} 100 0 0
				foot
		glPopMatrix
	glPopMatrix

	# Flush any remaining drawing commands and flip the buffers.
	glFlush
	cglSwapBuffers
}


# Set the angles of the current robot's joints by averaging angles from a
# "before" and "after" position.  "Progress" indicates how far the joint
# has come from the "before" position toward the "after" one.
# This function stores the results directly into the Robot global variable.
#
SetJoints() {
	local FromAng=$1 ToAng=$2 Progress=$3;
	for ((i=0; i<13; i++)); do
		(( Robot_Joints[i]=$FromAng[i]+($ToAng[i]-$FromAng[i])*Progress/100 ));
	done
}


# Animate the robot by progressing it through the four walking states.
# This function renews the GLut timer so that it will be called again
#  periodically.
# The parameter is ignored.
#
Animate() {
	if [[ -n "$Robot_Animate" ]]; then
		let Robot_MoveProgress+=5;
		if (( Robot_MoveProgress >= 400 )); then
 			let Robot_MoveProgress-=400;
		fi
		(( idx1=Robot_MoveProgress/100 ))
		(( idx2=idx1+1 ))
		if (( idx2 > 3 )); then idx2=0; fi
		SetJoints "WalkScript$idx1" "WalkScript$idx2" $((Robot_MoveProgress - (idx1 * 100) ))
	else
		SetJoints "Standing" "Standing" 0
	fi
}

# Initialize the globals and set up OpenGL.
#
Init() {
	# Use fixed point numbers for all floating-point GL parameters
	cglPushDivisor 100

	# Turn on normalization of surface vectors and enable Z-buffering.
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	# Setup lighting
	# Turn on lights and iterate through the global array of light records.
	# For each light in the array enable that number light in OpenGl and
	#  set the colors for it.
	glEnable GL_LIGHTING
	glLight GL_LIGHT0 GL_AMBIENT 80 80 80 100
	glLight GL_LIGHT0 GL_DIFFUSE 80 80 80 100
	glLight GL_LIGHT0 GL_SPECULAR 80 80 80 100
	glLight GL_LIGHT0 GL_POSITION 1000 1000 1000 100
	glEnable GL_LIGHT0

	# Setup material properties.
	# Allocate quadrics with filled drawing style
	gluNewQuadric quadric
	gluQuadricDrawStyle quadric GLU_FILL

	build_head;
	build_torso;
	build_lower_torso;
	build_upper_arm;
	build_lower_arm;
	build_upper_leg;
	build_lower_leg;
	build_foot;

	# Initialize runtime variables.
	Robot_MoveProgress=0;
	SetJoints "WalkScript0" "WalkScript0" 0
	Robot_Animate=true;
}

RenderLoop_DispatchEvent() {
	if ! ModelViewer_DispatchEvent "$@"; then
		if [[ "$1" == "K" && "$2" == "+" && "$3" == "q" ]]; then
			RenderLoop_Done=1;
		fi
	fi
}

RenderLoop_Render() {
	ModelViewer_Update
	Animate
	Repaint
}

main() {
	Init
	RenderLoop_Run;
	cglQuit
}

# TODO: Write better options processing
if (( $# > 1 )) && [[ "$1" == "--geometry" ]]; then
        CmdlineGL_Options=("${CmdlineGL_Options[@]}" "$1" "$2");
        shift;
        shift;
fi
if (( ! $# )); then
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
	echo 'Usage: Robot.sh [ --record | --dump ]'
	echo
	echo '   --dump    Dump all output to stdout at a virtual 40fps'
	echo '   --record  Run CmdlineGL, but duplicate all output to "./replay"'
	echo
	echo '   Recordings can be played by piping them into CmdlineGL.'
	echo '   For instance:'
	echo '         $ CmdlineGL <replay >/dev/null'
fi

