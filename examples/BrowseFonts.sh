#! /bin/bash
#
#  This example displays each font in the /usr/share directory by
#  rendering it as an extruded font.  Iterate through the list with
#  '[' and ']' keys.
#
text="$1";
set -eu

source CmdlineGL.lib
CmdlineGL_LoadLib RenderLoop ModelViewer

fonts=( `find /usr/share -name '*.ttf' | grep -i mono` )
font_n=${#fonts[@]}
font_i=

swap_font() {
	if [[ -n "$font_i" ]]; then ftglDestroyFont font1; else font_i=0; true; fi
	echo "${fonts[font_i]}"
	# Load font file and configure font rendering parameters
	ftglCreateExtrudeFont font1 "${fonts[font_i]}"
	ftglSetFontFaceSize   font1 72 72
	ftglSetFontDepth      font1 20
}

next_font() {
	if (( font_i + 1 < font_n )); then
		let ++font_i
		swap_font
	fi
}
prev_font() {
	if (( font_i > 0 )); then
		let font_i--
		swap_font
	fi
}

Init() {
	# Initialize CmdlineGL for rendering only (no input or feedback)
	glEnable GL_NORMALIZE GL_DEPTH_TEST GL_CULL_FACE
	glShadeModel GL_SMOOTH

	# set up lighting (otherwise no change as it rotates)
	glEnable GL_LIGHTING GL_LIGHT0
	glLight GL_LIGHT0 GL_AMBIENT .8 .8 .8 0
	glLight GL_LIGHT0 GL_DIFFUSE 1 .8 .8 0
	glLight GL_LIGHT0 GL_SPECULAR .8 .8 .8 0
	glLight GL_LIGHT0 GL_POSITION 10 10 10 1
	swap_font
}

RenderLoop_Render() {
	ModelViewer_Update
	
	glLoadIdentity
	ModelViewer_ApplyMatrix
	
	glTranslate -40 0 0
	glScale 1/40
	glColor 0.5 0.5 0.5 1
	ftglRenderFont font1 "${fonts[font_i]}" FTGL_RENDER_ALL
	
	glFlush
	cglSwapBuffers
	glClear GL_COLOR_BUFFER_BIT GL_DEPTH_BUFFER_BIT
}

RenderLoop_DispatchEvent() {
	if ! ModelViewer_DispatchEvent "$@"; then
		if [[ "$1" == "K" && "$2" == "+" && "$3" == q ]]; then
			RenderLoop_Done=1
		elif [[ "$1" == "K" && "$2" == "+" && "$3" == ']' ]]; then
			next_font
		elif [[ "$1" == "K" && "$2" == "+" && "$3" == '[' ]]; then
			prev_font
		else
			true
		fi
	fi
}

CmdlineGL_Start rw || die "Can't init CmdlineGL"
Init
RenderLoop_Run
cglQuit
