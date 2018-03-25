#! /bin/bash
#
#  This is an extremely minimal example which rotates a string of text
#  rendered in 3D.  The only complicated part is finding a valid font...
#
text="$1";
font="$2";
set -eu

source "${BASH_SOURCE%/*}/../CmdlineGL.lib" || die "Can't find CmdlineGL.lib (${BASH_SOURCE%/*}/../CmdlineGL.lib)";

if [[ -z "$text" ]]; then
	echo "Usage: SpinText.sh STRING_OF_TEXT [FONT_FILE]";
	exit 1;
fi
if [[ -z "$font" ]]; then
	# See if we can find a sensible default
	font=`find /usr/share -name '*.ttf' | grep -i mono | head -n 1`;
	if [[ ! -f "$font" ]]; then
		echo "Can't find any default font; specify font filename as second argument.";
		exit 2;
	fi
else
	if [[ ! -f "$font" ]]; then
		echo "No such font \"$font\"";
		exit 2;
	fi
fi

R=0
T=0
spin_rate=12 # degrees per second

# Initialize CmdlineGL for rendering only (no input or feedback)
CmdlineGL_Start ro
glEnable GL_NORMALIZE GL_DEPTH_TEST GL_CULL_FACE
glShadeModel GL_SMOOTH

# Load font file and configure font rendering parameters
ftglCreateExtrudeFont font1 "$font"
ftglSetFontFaceSize   font1 72 72
ftglSetFontDepth      font1 20

# Prepare the graphics in a display list.  Need to call once first since ftgl
# creates its own display lists, then again to capture those in the second
# display list.
ftglRenderFont font1 "$text" FTGL_ALIGN_CENTER FTGL_RENDER_ALL
glNewList mytext GL_COMPILE
glTranslate -$(( ${#text}/2 * 40 )) -36 10   # flaky guess at midpoint of string
ftglRenderFont font1 "$text" FTGL_RENDER_ALL
glEndList

# set up lighting (otherwise no change as it rotates)
glEnable GL_LIGHTING GL_LIGHT0
glLight GL_LIGHT0 GL_AMBIENT .8 .8 .8 0
glLight GL_LIGHT0 GL_DIFFUSE 1 .8 .8 0
glLight GL_LIGHT0 GL_SPECULAR .8 .8 .8 0
glLight GL_LIGHT0 GL_POSITION 10 10 10 1

while true; do
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
	glLoadIdentity
	glRotate $((R+=spin_rate))/60 0 1 0  # assuming 60fps
	glScale 10/$((40 * ${#text} / 2))    # flaky guess at scaling to window width
	glCallList mytext
	glFlush
	cglSwapBuffers
	cglSync $((T+=16)) # blindly assume we can maintain 60fps
done
