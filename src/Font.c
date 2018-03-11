#include <config.h>
#include "Global.h"
#include "ParseGL.h"
#include "ProcessInput.h"
#include "SymbolHash.h"

#ifdef HAVE_LIBFTGL

/*=head2 FTGL Font Functions

These functions come from the FTGL library.  They can open any font file that the FreeType
library can open.  A symbolic name takes the place of FTGL's C<FTfont*> pointer. You must
call one of the Create methods to initialize a symbolic name (and it must succeed).

=item ftglCreateBitmapFont NAME "FONT_FILE_PATH"

Create a 2-color font that renders directly to the framebuffer in 2D.

=item ftglCreatePixmapFont NAME "FONT_FILE_PATH"

Create a 256-color-grayscale font that renders directly to the framebuffer in 2D.

=item ftglCreateTextureFont NAME "FONT_FILE_PATH"

Create a font that renders each glyph into a texture, so that a small set of textures can
supply all your rendering needs.

=item ftglCreateBufferFont NAME "FONT_FILE_PATH"

Create a texture-based font that renders each line of text into its own texture, so that common
phrases can be drawn as a single polygon.

=item ftglCreateExtrudeFont NAME "FONT_FILE_PATH"

Create 3D models to represent each glyph extruded as a solid object.

=item ftglCreateOutlineFont NAME "FONT_FILE_PATH"

Create 2D model of GL lines that trace the outline of each glyph.

=item ftglCreatePolygonFont NAME "FONT_FILE_PATH"

Create each glyph as a flat 2D mesh of polygons.

=item ftglDestroyFont NAME

Free resources and un-define the symbolic NAME.

=cut */

bool InitNamedFont(SymbVarEntry *sym, FTGLfont *font) {
	if (!font) {
		/* if newly created, remove the tree node so we don't have a null pointer dangling around */
		if (!sym->Data)
			DeleteSymbVar(sym);
		return false;
	}
	/* If the font already existed, delete the old one */
	if (sym->Data)
		ftglDestroyFont( (FTGLfont*) sym->Data );
	
	sym->Data= font;
	return true;
}

COMMAND(ftglCreateBitmapFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreateBitmapFont(parsed->strings[0]));
}
COMMAND(ftglCreateBufferFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreateBufferFont(parsed->strings[0]));
}
COMMAND(ftglCreateExtrudeFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreateExtrudeFont(parsed->strings[0]));
}
COMMAND(ftglCreateOutlineFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreateOutlineFont(parsed->strings[0]));
}
COMMAND(ftglCreatePixmapFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreatePixmapFont(parsed->strings[0]));
}
COMMAND(ftglCreatePolygonFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreatePolygonFont(parsed->strings[0]));
}
COMMAND(ftglCreateTextureFont, "F!/") {
	return InitNamedFont(parsed->objects[0], ftglCreateTextureFont(parsed->strings[0]));
}

COMMAND(ftglDestroyFont, "F") {
	SymbVarEntry *sym= parsed->objects[0];
	ftglDestroyFont( (FTGLfont*) sym->Data );
	DeleteSymbVar(sym);
	return true;
}

/*=item ftglSetFontCharMap NAME CMAP_CODE

Set the charmap to one of the codes known by the FreeType library.

=item ftglSetFontFaceSize NAME SIZE

For texture and raster fonts, render each glyph at SIZE pixels.  For Vector fonts, render each
glyph at SIZE logical units.

=item ftglSetFontDepth NAME SIZE

For extruded fonts, set the depth to SIZE units.

=item ftglSetFontOutset NAME FRONT BACK

For extruded fonts.

=cut */

COMMAND(ftglSetFontCharMap, "Fi") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	ftglSetFontCharMap(font, parsed->ints[0]);
	return true;
}

COMMAND(ftglSetFontFaceSize, "Fii") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	ftglSetFontFaceSize(font, parsed->ints[0], parsed->ints[1]);
	return true;
}

COMMAND(ftglSetFontDepth, "Ff") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	ftglSetFontDepth(font, parsed->floats[0]);
	return true;
}

COMMAND(ftglSetFontOutset, "Fff") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	ftglSetFontOutset(font, parsed->floats[0], parsed->floats[1]);
	return true;
}

/*=item ftglRenderFont NAME TEXT FLAGS[...]

Renter TEXT using NAMEd font, affected by FLAGS.  In the C API, the flags are combined, but in
this command each flag is given as another parameter.  Flags are: FTGL_RENDER_FRONT,
FTGL_RENDER_BACK, FTGL_RENDER_SIDE, FTGL_RENDER_ALL, FTGL_ALIGN_LEFT, FTGL_ALIGN_RIGHT,
FTGL_ALIGN_CENTER, FTGL_ALIGN_JUSTIFY.

=cut */

COMMAND(ftglRenderFont, "Fsi*") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	int flags= 0, i;
	for (i=0; i< parsed->iCnt; i++)
		flags |= parsed->ints[i];
	ftglRenderFont(font, parsed->strings[0], flags);
	return true;
}

#endif
