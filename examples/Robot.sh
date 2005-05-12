#*****************************************************************************\
# Project: Computer Graphics Final Exam                                       *
# Title:   Robot.cpp                                                          *
# Descrip: GLut-driven animated OpenGL rendering of a robot.                  *
# Author:  Michael Conrad                                                     *
#*****************************************************************************/

. ../bin/CmdlineGL_BashBindings

#---------------------------------------------------------------------------
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

ToInt() {
	echo ${1/./} | sed -r 's/(-?)0*([1-9]*[0-9])/\1\2/'
}
ToFloat() {
	if [ "${1:0:1}" = "-" ]; then ival=-0000${1#-}; else ival=0000$1; fi
	let dec_pos=${#ival}-$2;
	echo -n ${ival:0:$dec_pos}.${ival:$dec_pos};
}

HEAD_RADIUS=1.00;
TORSO_LENGTH=4.80;
TORSO_RADIUS=0.90;
HIP_WIDTH=2.20;
HIP_SPACING=0.20;
HIP_RADIUS=0.35;
SHOULDER_WIDTH=5.00;
SHOULDER_RADIUS=0.70;
UPPER_ARM_LENGTH=3.00;
UPPER_ARM_RADIUS=0.40;
LOWER_ARM_LENGTH=2.80;
LOWER_ARM_RADIUS=0.30;
UPPER_LEG_LENGTH=3.40;
UPPER_LEG_RADIUS=0.55;
LOWER_LEG_LENGTH=3.00;
LOWER_LEG_RADIUS=0.45;
FOOT_LENGTH=1.60;
FOOT_HEIGHT=0.30;
HEAD_HEIGHT=`ToFloat $((\`ToInt $TORSO_LENGTH\`+\`ToInt $HEAD_RADIUS\`)) 2`;
PELVIC_HEIGHT=`ToFloat $((-\`ToInt $HIP_RADIUS\` * 18)) 3`;
SHOULDER_HEIGHT=`ToFloat $((\`ToInt $TORSO_LENGTH\` * 84)) 4`;
SHOULDER_OFFSET=`ToFloat $((\`ToInt $SHOULDER_WIDTH\`/2 - \`ToInt $UPPER_ARM_RADIUS\` )) 2`;
ELBOW_RADIUS=`ToFloat $((\`ToInt $LOWER_ARM_RADIUS\`*12)) 3`;
ELBOW_WIDTH=`ToFloat $((\`ToInt $LOWER_ARM_RADIUS\`*24)) 3`;
LEG_OFFSET=`ToFloat $((\`ToInt $HIP_WIDTH\`/2 )) 2`;
UPPER_LEG_PIPE_LENGTH=`ToFloat $((\`ToInt $UPPER_LEG_LENGTH\`+\`ToInt $UPPER_LEG_RADIUS\`)) 2`;
HIP_SUPPORT_WIDTH=`ToFloat $((\`ToInt $HIP_WIDTH\` - \`ToInt $HIP_SPACING\` * 2 - \`ToInt $UPPER_LEG_RADIUS\` * 2)) 2`;
HIP_SUPPORT_HALF_WIDTH=`ToFloat $((\`ToInt $HIP_SUPPORT_WIDTH\`/2)) 2`;
KNEE_RADIUS=`ToFloat $((\`ToInt $LOWER_LEG_RADIUS\`*12)) 3`;
KNEE_WIDTH=`ToFloat $((\`ToInt $LOWER_LEG_RADIUS\`*24)) 3`;

# Indicies of important angles
#
NeckX=0; NeckY=1;
LShoulder=2; RShoulder=3; LElbow=4; RElbow=5;
Torso=6;
LHip=7; RHip=8; LKnee=9; RKnee=10; LFoot=11; RFoot=12;

# Joint records for the motionless robot and the four stage animation
#
Standing=(    000  000    010  010 -020 -020    000    000  000  000  000  010  010 );
WalkScript0=( 000  000    020  020 -055 -055    000   -025  010  070  010 -020 -005 );
WalkScript1=( 000  000    040  000 -040 -070   -007   -030  025  020  030  020  030 );
WalkScript2=( 000  000    020  020 -055 -055    000    010 -025  010  070 -005 -020 );
WalkScript3=( 000  000    000  040 -070 -040    007    025 -030  030  020  030  020 );


# Variables that describe the current robot
#
Robot_Joints=0;
Robot_MoveProgress=0;
Robot_Animate=true;

# Variables related to the camers
#
View_Direction=0;
View_Pitch=0;
View_Mode=0;

# Variables related to the user interface
#
Mouse_LastX=0;
Mouse_LastY=0;

# Draw the head of the robot.
# Draws a sphere with radius "HEAD_RADIUS" at the current origin.
#
build_head() {
	glNewList head GL_COMPILE
	glPushMatrix
	glScale $HEAD_RADIUS $HEAD_RADIUS $HEAD_RADIUS
	gluSphere quadric 1.0 10.0 10.0
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
	gluCylinder $1 0.0 $baseRadius 0.0 $slices 1.0
	gluCylinder $1 $baseRadius $topRadius $height $slices $stacks
	glTranslate 0.0 0.0 $height
	gluCylinder $1 $topRadius 0.0 0.0 $slices 1.0
	glTranslate 0.0 0.0 -$height
}

# Draw the torso of the robot.
# This consists of a large vertical cylinder above the origin a tapered
# cylinder on the bottom of it and horizontal shoulder cylinder.
# The origin ends up located at the very bottom of the torso.
#
build_torso() {
	glNewList torso GL_COMPILE
	glPushMatrix
	glTranslate 0.0 $TORSO_LENGTH 0.0
	glRotate 90.0 1.0 0.0 0.0
	local TorsoZ=$((`ToInt $TORSO_LENGTH` * 8));
	closedCylinder quadric $TORSO_RADIUS $TORSO_RADIUS `ToFloat $TorsoZ 3` 20.0 1.0
	glTranslate 0.0 0.0 `ToFloat $TorsoZ 3`
	closedCylinder quadric $TORSO_RADIUS `ToFloat $((\`ToInt $HIP_SUPPORT_WIDTH\`/2)) 2` `ToFloat $((\`ToInt $TORSO_LENGTH\`*2)) 3` 20.0 1.0
	glPopMatrix

	glPushMatrix
	glTranslate -`ToFloat $((\`ToInt $SHOULDER_WIDTH\`/2)) 2` $SHOULDER_HEIGHT 0.0
	glRotate 90.0 0.0 1.0 0.0
	closedCylinder quadric $SHOULDER_RADIUS $SHOULDER_RADIUS $SHOULDER_WIDTH 20.0 1.0
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
	glTranslate 0.0 $PELVIC_HEIGHT 0.0
	glRotate 90.0 0.0 1.0 0.0
	glTranslate 0.0 0.0 -$HIP_SUPPORT_HALF_WIDTH
	closedCylinder quadric $TORSO_RADIUS $TORSO_RADIUS $HIP_SUPPORT_WIDTH 20.0 1.0

	glTranslate 0.0 0.0 -`ToFloat $((\`ToInt $HIP_SPACING\` + \`ToInt $UPPER_LEG_RADIUS\`)) 2`
	gluCylinder quadric $HIP_RADIUS $HIP_RADIUS $HIP_WIDTH 10.0 1.0
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
	glRotate 90.0 1.0 0.0 0.0
	gluCylinder quadric $UPPER_ARM_RADIUS $LOWER_ARM_RADIUS $UPPER_ARM_LENGTH 20.0 1.0
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
	glRotate 90.0 0.0 1.0 0.0
	glTranslate 0.0 0.0 -$ELBOW_RADIUS # this is actually ELBOW_WIDTH/2
	closedCylinder quadric $ELBOW_RADIUS $ELBOW_RADIUS $ELBOW_WIDTH 20.0 1.0
	glPopMatrix

	glPushMatrix
	glRotate 90.0 1.0 0.0 0.0
	closedCylinder quadric $LOWER_ARM_RADIUS $LOWER_ARM_RADIUS $LOWER_ARM_LENGTH 20.0 1.0
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
	glTranslate 0.0 $UPPER_LEG_RADIUS 0.0
	glRotate 90.0 1.0 0.0 0.0
	closedCylinder quadric $UPPER_LEG_RADIUS $LOWER_LEG_RADIUS $UPPER_LEG_PIPE_LENGTH 20.0 1.0
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
	glRotate 90.0 0.0 1.0 0.0
	glTranslate 0.0 0.0 -$KNEE_RADIUS # this is actually knee_width/2
	closedCylinder quadric $KNEE_RADIUS $KNEE_RADIUS $KNEE_WIDTH 20.0 1.0
	glPopMatrix

	glPushMatrix
	glRotate 90.0 1.0 0.0 0.0
	gluCylinder quadric $LOWER_LEG_RADIUS $LOWER_LEG_RADIUS $LOWER_LEG_LENGTH 20.0 1.0
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
	glRotate 90.0 0.0 1.0 0.0
	glTranslate 0.0 0.0 -$LOWER_LEG_RADIUS
	closedCylinder quadric $LOWER_LEG_RADIUS $LOWER_LEG_RADIUS `ToFloat $((\`ToInt $LOWER_LEG_RADIUS\`*2 )) 2` 20.0 1.0
	glPopMatrix
	glPushMatrix
	glTranslate 0.0 -`ToFloat $((\`ToInt $LOWER_LEG_RADIUS\` * 3)) 3` 0.0
	glScale 1.0 0.3 1.0
	closedCylinder quadric `ToFloat $((\`ToInt $LOWER_LEG_RADIUS\`*8)) 3` `ToFloat $((\`ToInt $LOWER_LEG_RADIUS\`*12)) 3` $FOOT_LENGTH 20.0 1.0
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
	glTranslate 0.0 0.0 -11.0
	glRotate $View_Pitch 1.0 0.0 0.0
	glRotate $View_Direction 0.0 1.0 0.0

	glPushMatrix
	# Move to the center of the head rotate by the neck angles and draw.
	glTranslate 0.0 $HEAD_HEIGHT 0.0
	glRotate ${Robot_Joints[$NeckX]} 1.0 0.0 0.0
	glRotate ${Robot_Joints[$NeckY]} 0.0 1.0 0.0
		head
	glPopMatrix

	# Draw the torso.  It never needs rotated.
	torso

	glPushMatrix
	# Draw the left arm.  Start at the left shoulder draw the uper arm
	#  then translate down to the elbow rotate and draw the lower arm.
	glTranslate $SHOULDER_OFFSET $SHOULDER_HEIGHT 0.0
	glRotate ${Robot_Joints[$LShoulder]} 1.0 0.0 0.0
		upper_arm
		glTranslate 0.0 -$UPPER_ARM_LENGTH 0.0
		glRotate ${Robot_Joints[$LElbow]} 1.0 0.0 0.0
			lower_arm
	glPopMatrix

	glPushMatrix
	# Same for the right arm.
	glTranslate  -$SHOULDER_OFFSET $SHOULDER_HEIGHT 0.0
	glRotate ${Robot_Joints[$RShoulder]} 1.0 0.0 0.0
		upper_arm
		glTranslate 0.0 -$UPPER_ARM_LENGTH 0.0
		glRotate ${Robot_Joints[$RElbow]} 1.0 0.0 0.0
			lower_arm
	glPopMatrix

	glPushMatrix
	# Rotate for the hips draw them then draw the left and right leg.
	glRotate ${Robot_Joints[$Torso]} 0.0 1.0 0.0
		lower_torso

		glPushMatrix
		# First move to the left draw the upper leg and then translate down
		#  to the knee draw the lower leg then translate down to the foot
		#  then rotate and draw it.
		glTranslate $LEG_OFFSET $PELVIC_HEIGHT 0.0
		glRotate ${Robot_Joints[$LHip]} 1.0 0.0 0.0
			upper_leg
			glTranslate 0.0 -$UPPER_LEG_LENGTH 0.0
			glRotate ${Robot_Joints[$LKnee]} 1.0 0.0 0.0
				lower_leg
				glTranslate 0.0 -$LOWER_LEG_LENGTH 0.0
				glRotate ${Robot_Joints[$LFoot]} 1.0 0.0 0.0
				foot
		glPopMatrix

		glPushMatrix
		# Same for the right leg.
		glTranslate -$LEG_OFFSET $PELVIC_HEIGHT 0.0
		glRotate ${Robot_Joints[$RHip]} 1.0 0.0 0.0
			upper_leg
			glTranslate 0.0 -$UPPER_LEG_LENGTH 0.0
			glRotate ${Robot_Joints[$RKnee]} 1.0 0.0 0.0
				lower_leg
				glTranslate 0.0 -$LOWER_LEG_LENGTH 0.0
				glRotate ${Robot_Joints[$RFoot]} 1.0 0.0 0.0
				foot
		glPopMatrix
	glPopMatrix

	glPopMatrix

	# Flush any remaining drawing commands and flip the buffers.
	glFlush
	glutSwapBuffers
}


# Set the angles of the current robot's joints by averaging angles from a
# "before" and "after" position.  "Progress" indicates how far the joint
# has come from the "before" position toward the "after" one.
# This function stores the results directly into the Robot global variable.
#
SetJoints() {
	local FromAng=$1;
	local ToAng=$2;
	local Progress=$3;
	local Angle;
	for i in 0 1 2 3 4 5 6 7 8 9 10 11 12; do
		eval "Angle=\$(( \${$FromAng[$i]}*10 + ( \${$ToAng[$i]} - \${$FromAng[$i]} ) * $Progress / 10))";
		let dec_pos=${#Angle}-1;
		Robot_Joints[$i]=$(($Angle / 10)).${Angle:$dec_pos:1};
	done
}


# Animate the robot by progressing it through the four walking states.
# This function renews the GLut timer so that it will be called again
#  periodically.
# The parameter is ignored.
#
Animate() {
	if [ -n "$Robot_Animate" ]; then
		let Robot_MoveProgress+=1;
		if [ $Robot_MoveProgress -gt 400 ]; then
 			let Robot_MoveProgress-=400;
		fi
		let idx1=$Robot_MoveProgress/100;
		let idx2=$idx1+1;
		if [ $idx2 -gt 3 ]; then idx2=0; fi
		SetJoints "WalkScript$idx1" "WalkScript$idx2" $(($Robot_MoveProgress - ($idx1 * 100) ))
		Repaint
	else
		SetJoints "Standing" "Standing" 0
	fi
}


# Handle mouse clicks.
# If the left button is clicked increase the angle of the active joint
# If the right button is clicked decrease the angle of the active joint
# Repaint the image.
# Also record the position of the mouse as the "last position".
#
#mouse(int btn int state int x int y) {
#	Mouse_LastX= x;
#	Mouse_LastY= y;
#}


# Handle mouse drag actions.
# If the mouse has moved since last time change the pitch or direction
# by the vertical or horizontal distance the mouse has moved.
# Also repaint the robot and record the "last position" of the mouse.
#
#mouseMotion(int x int y) {
#	int dx= Mouse_LastX - x;
#	int dy= Mouse_LastY - y;
#
#	if (dy != 0) {
#		View.Pitch-= dy;
#	}
#	if (dx != 0) {
#		View.Direction+= dx;
#	}
#
#	if (!Robot.Animate) display(
#	Mouse_LastX= x;
#	Mouse_LastY= y;
#}

# Initialize the globals and set up OpenGL.
#
Init() {
	# Turn on normalization of surface vectors and enable Z-buffering.
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	# Setup lighting
	# Turn on lights and iterate through the global array of light records.
	# For each light in the array enable that number light in OpenGl and
	#  set the colors for it.
	glEnable GL_LIGHTING
	glLight GL_LIGHT0 GL_AMBIENT 0.8 0.8 0.8 1.0
	glLight GL_LIGHT0 GL_DIFFUSE 0.8 0.8 0.8 1.0
	glLight GL_LIGHT0 GL_SPECULAR 0.8 0.8 0.8 1.0
	glLight GL_LIGHT0 GL_POSITION 10 10 10 1
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
	View_Direction=0.0;
	View_Pitch=0.0;
	View_Mode=0;
	Robot_Animate=true;
}

ProcessInput() {
	return 0;
}

# Start up and initialize various GLut things.
# Then turn control over to GLut for the remainder of the program.
#
main() {
	# Initialize our own stuff (and OpenGL lighting).
	Init;

	# repaint robot forever
	while ProcessInput; do
		Animate;
	done
}

main
