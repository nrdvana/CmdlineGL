#ifndef FONT_H
#define FONT_H

#include <SDL.h>
#include "Global.h"
#include "GlHeaders.h"

typedef struct CharCoordts_t {
	double X0t, X1t, Y0t, Y1t; // texture coordinates, normalized 0..1 over the texture they come from
	double X0v, X1v, Y0v, Y1v; // vertex coordinates, offsets from the bottom left corner of the character box
	double BoxWidth, BoxHeight; // dimensions used for character spacing
} CharCoordts;

#define FONT_CHAR_MAX 96
typedef struct Font_t {
	CharCoordts Chars[FONT_CHAR_MAX];
	GLuint Texture;
	GLuint DisplayList0;
} Font;

Font* Font_Initialize(void *this);
void* Font_Finalize(Font* this);

#define CGL_BMPFONT 1

void WriteText(Font *F, const char *Text);
bool GenerateFont(SDL_Surface *Img, Font *F);

PUBLISHED(cglText,DoText);

#endif
