#include "Global.h"
#include "ParseGL.h"
#include "ProcessInput.h"
#include "SymbolHash.h"
#include "FTGL/ftgl.h"

COMMAND(ftglCreateTextureFont, "F!/") {
	SymbVarEntry *sym= argv[0].as_sym;
	FTGLfont *font= ftglCreateTextureFont(argv[1].as_str);
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
	ftglDestroyFont( (FTGLfont*) argv[0].as_sym->Data );
}