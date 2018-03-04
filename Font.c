#include "Global.h"
#include "ParseGL.h"
#include "ProcessInput.h"
#include "SymbolHash.h"
#include "GlHeaders.h"

#ifdef HAVE_LIBFTGL

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

COMMAND(ftglRenderFont, "Fsi*") {
	FTGLfont *font= (FTGLfont*) parsed->objects[0]->Data;
	int flags= 0, i;
	for (i=0; i< parsed->iCnt; i++)
		flags |= parsed->ints[i];
	ftglRenderFont(font, parsed->strings[0], flags);
	return true;
}

#endif
