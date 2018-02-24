#! /bin/bash
[ -n "$BASH_VERSION" ] || exec bash $0

# Define our handy die function
die() { echo "$@" >&2; exit 2; }
set -ux
# Load bash libraries
source "${BASH_SOURCE%/*}/../share/CmdlineGL.lib" || die "Can't find ../share directory (from $PWD via ${BASH_SOURCE%/*})";

FixedPt=1000
CmdlineGL_LoadLib RenderLoop Geom Ship LaserBeam Cube

let MAX_LASER_TRAVEL=100*$FixedPt
LASER_SPEED=100
SHOOT_PERIOD=200
MAX_LASERS=100
HighestInitLaser=-1

Init() {
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	glEnable GL_TEXTURE_2D
	glEnable GL_CULL_FACE
	glShadeModel GL_SMOOTH
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT

	echo "Loading models..." >&2
	LaserBeam_InitGfx
	Ship_InitGfx
	Cube_InitGfx
	BuildCubeField

	echo "Initializing game state..." >&2
	SetLights
	glEnable GL_NORMALIZE
	glEnable GL_DEPTH_TEST
	glEnable GL_TEXTURE_2D
	glEnable GL_CULL_FACE
	glFog GL_FOG_MODE GL_LINEAR
	glFog GL_FOG_COLOR '#333333'
	glClearColor '#333333'
	glFog GL_FOG_START 10000
	glFog GL_FOG_END 100001
	glFog GL_FOG_DENSITY 100
	glEnable GL_FOG
	glEnable GL_LIGHTING

	InitCoordSys Ship
	InitCoordSys Cam
	Ship_IV_Scale -1000
	Ship_JV_Scale -1000
	Ship_KV_Scale -1000
	InitVec CamTrail 0 0 0
	ShipSpeed=1
	((CamDist=6*FixedPt))
	CamFollowHeight=1
	ResetCam
	InpAimLf=0
	InpAimRt=0
	InpAimUp=0
	InpAimDn=0
	InpAccel=0
	InpDeaccel=0
	InpShoot=0
	LaserCount=0
	ShootCount=0
	NextGun=0
	let LastShoot=Timing_T
}

SetLights() {
	glLoadIdentity
	glEnable GL_LIGHTING
	glEnable GL_COLOR_MATERIAL
	glEnable GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT 0800 0800 0800 0
 	glLight GL_LIGHT0 GL_DIFFUSE 1000 0800 0800 0
	glLight GL_LIGHT0 GL_SPECULAR 0800 0800 0800 0
	glLight GL_LIGHT0 GL_POSITION 10000 10000 10000 1000
}

BuildCubeField() {
	glNewList CubeField GL_COMPILE
	glTranslate -25000 -25000 -25000
	for (( x=0; x<10; x++)); do
		for (( y=0; y<10; y++)); do
			for (( z=0; z<10; z++)); do
				Cube
				glTranslate 0 0 5000
			done
			glTranslate 0 0 -50000
			glTranslate 0 5000 0
		done
		glTranslate 0 -50000 0
		glTranslate 5000 0 0
	done
	glEndList
}

DrawCoordinates() {
	glBegin GL_LINES
	glColor "#FF0000"
	glVertex 1000 0 0
	glVertex 0 0 0
	glColor "#00FF00"
	glVertex 0 1000 0
	glVertex 0 0 0
	glColor "#0000FF"
	glVertex 0 0 1000
	glVertex 0 0 0
	glEnd
}

InitLaser() {
	InitCoordSys ${1}
	let ${1}_Travel=0
}

AddLaser() {
	let idx=LaserCount++
	if ((idx>HighestInitLaser)); then InitLaser Laser${idx}; ((HighestInitLaser=idx)); fi
	CoordSys_Clone Laser${idx} $4
	((Laser${idx}_Pos_x=$1, Laser${idx}_Pos_y=$2, Laser${idx}_Pos_z=$3, Laser${idx}_Travel=0))
}

CloneLaser() {
	CoordSys_Clone $1 $2
	((Laser${1}_Travel=Laser${2}_Travel))
}

RemoveLaser() {
	let LaserCount--
	CloneLaser Laser$1 Laser$LaserCount
}

eval "UpdateLasers() {
	local progress=\$Timing_dT*$LASER_SPEED
	for ((i=LaserCount-1; i>=0; i--)); do
		((Laser\${i}_Pos_x+= Laser\${i}_KV_x*progress/$FixedPt))
		((Laser\${i}_Pos_y+= Laser\${i}_KV_y*progress/$FixedPt))
		((Laser\${i}_Pos_z+= Laser\${i}_KV_z*progress/$FixedPt))
		((Laser\${i}_Travel+=progress))
		if ((Laser\${i}_Travel>$MAX_LASER_TRAVEL)); then
			RemoveLaser \$i
		fi
	done
}"

DrawLasers() {
	glDisable GL_LIGHTING
	for ((i=0; i<LaserCount; i++)); do
		glPushMatrix
		Laser${i}_EnterCS
		LaserBeam
		glPopMatrix
	done
	glEnable GL_LIGHTING
}

Shoot() {
	local x y z xOfs yOfs zOfs
	((xOfs=Ship_GunXOffset[NextGun], yOfs=Ship_GunYOffset, zOfs=Ship_GunZOffset,
	 x=Ship_Pos_x+(Ship_IV_x*xOfs+Ship_JV_x*yOfs+Ship_KV_x*zOfs)/FIXEDPT,
	 y=Ship_Pos_y+(Ship_IV_y*xOfs+Ship_JV_y*yOfs+Ship_KV_y*zOfs)/FIXEDPT,
	 z=Ship_Pos_z+(Ship_IV_z*xOfs+Ship_JV_z*yOfs+Ship_KV_z*zOfs)/FIXEDPT))
	AddLaser $x $y $z Ship
#	cat ${SOUNDDIR:-.}/laser.dsp >/dev/dsp &
	((NextGun++, NextGun>3?NextGun=0:0))
}

# Pretty simple- update the ship's direction and speed, then move it along
# its forward vector.
#
# Create a new bullet if it's time and the spacebar is pressed.
#
UpdateShip() {
	local dT=Timing_dT Dist
	if ((InpAimLf)); then Ship_RelativeYaw $((dT*3)); Ship_RelativeRoll $((-dT*2)); fi
	if ((InpAimRt)); then Ship_RelativeYaw $((-dT*3)); Ship_RelativeRoll $((dT*2)); fi
	if ((InpAimUp)); then Ship_RelativePitch $((dT*6));  fi
	if ((InpAimDn)); then Ship_RelativePitch $((-dT*6)); fi
	Ship_Normalize

	if ((InpAccel && ShipSpeed<20)); then let ShipSpeed++; fi
	if ((InpDeaccel && ShipSpeed>0)); then let ShipSpeed--; fi
	((Dist=Timing_dT*ShipSpeed))
	((Ship_Pos_x+=Ship_KV_x*Dist/$FixedPt, Ship_Pos_y+=Ship_KV_y*Dist/$FixedPt, Ship_Pos_z+=Ship_KV_z*Dist/$FixedPt))

	if ((InpShoot||ShootCount)); then
		((Timing_T-LastShoot>SHOOT_PERIOD*2? LastShoot=Timing_T-SHOOT_PERIOD*2:0))
		for ((; LastShoot+SHOOT_PERIOD<Timing_T; LastShoot+=SHOOT_PERIOD)); do
			Shoot
		done
		ShootCount=0
	fi
}

# The general idea behind this one is to find the vector between the camera
# and the ship, and set it to exactly $CamDist, moving the camera along
# the vector as necessary.  The usual effect is for the camera to
# "fall in line" behind the ship.
#
# The camera's upward vector is always the same as the ship, and the camera
# looks toward 19 units ahead of the ship (so the player can see where they're
# going).
#
UpdateCam() {
	local fx fy fz # The "follow" point, toward which the camera travels
	((fx=Ship_Pos_x+Ship_JV_x*CamFollowHeight, fy=Ship_Pos_y+Ship_JV_y*CamFollowHeight, fz=Ship_Pos_z+Ship_JV_y*CamFollowHeight))
	((CamTrail_x=Cam_Pos_x-fx, CamTrail_y=Cam_Pos_y-fy, CamTrail_z=Cam_Pos_z-fz))
	# avoid div-by-0
	if ((CamTrail_x*CamTrail_x+CamTrail_y*CamTrail_y+CamTrail_z*CamTrail_z>$FixedPt)); then
		CamTrail_SetMagnitude $CamDist
		((Cam_Pos_x=fx+CamTrail_x, Cam_Pos_y=fy+CamTrail_y, Cam_Pos_z=fz+CamTrail_z))
	else
		((Cam_Pos_x=fx-Ship_KV_x*CamDist, Cam_Pos_y=fy-Ship_KV_y*CamDist, Cam_Pos_z=fz-Ship_KV_z*CamDist))
	fi
	((Cam_KV_x=CamTrail_x-Ship_KV_x*19, Cam_KV_y=CamTrail_y-Ship_KV_y*19, Cam_KV_z=CamTrail_z-Ship_KV_z*19))
	Cam_KV_Normalize
	((Cam_JV_x=Ship_JV_x, Cam_JV_y=Ship_JV_y, Cam_JV_z=Ship_JV_z))
	Cam_RegenIV
	Cam_IV_Normalize
	Cam_RegenJV # the camera's "up" is not necessarily the same as the ship's "up"
}

# Put the camera directly on top of its follow point, which causes the Update to reset its location
ResetCam() {
	((Cam_Pos_x=Ship_Pos_x+Ship_JV_x*CamFollowHeight, Cam_Pos_y=Ship_Pos_y+Ship_JV_y*CamFollowHeight, Cam_Pos_z=Ship_Pos_z+Ship_JV_y*CamFollowHeight))
	UpdateCam
}

RenderLoop_DispatchEvent() {
	case "$1" in
	K)
		if [[ $2 = "+" ]]; then Press=1; else Press=0; fi
		case "$3" in
		right) InpAimRt=$Press;;
		left)  InpAimLf=$Press;;
		up)    InpAimUp=$Press;;
		down)  InpAimDn=$Press;;
		=)     InpAccel=$Press;;
		-)     InpDeaccel=$Press;;
		space) ((InpShoot=Press, Press?ShootCount++:0));;
		q)     RenderLoop_Done=1;;
		esac
		;;
	esac
}

RenderLoop_Render() {
	UpdateLasers
	UpdateShip
	UpdateCam
	
	glLoadIdentity
	Cam_ExitCS
	glPushMatrix
		glLight GL_LIGHT0 GL_POSITION 0 0 1 0
		glScale 5000 5000 5000
		glCallList CubeField
	glPopMatrix
	glPushMatrix
		Ship_EnterCS
	#	glTranslate 0 -1000 -5000
		Ship
	glPopMatrix
	DrawLasers
	sleep 1;
}

main() {
	Init
	RenderLoop_Run;
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
	echo 'Usage: FlightSim.sh [ --record | --dump ]'
	echo
	echo '   --dump    Dump all output to stdout at a virtual 40fps'
	echo '   --record  Run CmdlineGL, but duplicate all output to "./replay"'
	echo
	echo '   Recordings can be played by piping them into CmdlineGL.'
	echo '   For instance:'
	echo '         $ CmdlineGL <replay >/dev/null'
fi
