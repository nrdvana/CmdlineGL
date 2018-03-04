#include "Global.h"
#include "ParseGL.h"
#include "ProcessInput.h"
#include "SymbolHash.h"
#include "FTGL/ftgl.h"

COMMAND(ftglCreateTextureFont, "F!/") {
	SymbVarEntry *sym= parsed->objects[0];
	FTGLfont *font= ftglCreateTextureFont(parsed->strings[0]);
	if (!font) {
		/* if newly created, remove the tree node so we don't have a null pointer dangling around */
		if (!sym->Data)
			DeleteSymbVar(sym);
		return false;
	}
	
	if (sym->Data)
		ftglDestroyFont( (FTGLfont*) sym->Data );
	
	sym->Data= font;
	return true;
}

COMMAND(ftglDestroyFont, "F") {
	SymbVarEntry *sym= parsed->objects[0];
	ftglDestroyFont( (FTGLfont*) sym->Data );
	DeleteSymbVar(sym);
}
