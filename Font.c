#include <SDL.h>
#include "Global.h"
#include "GlHeaders.h"
#include "ParseGL.h"
#include "Font.h"

bool IsColMarker(Uint8* Pixel, SDL_PixelFormat *fmt) {
	int r, g, b;
	Uint32 color= *((Uint32*)Pixel);
	r= (color & fmt->Rmask) >> (fmt->Rshift - fmt->Rloss);
	g= (color & fmt->Gmask) >> (fmt->Gshift - fmt->Gloss);
	b= (color & fmt->Bmask) >> (fmt->Bshift - fmt->Bloss);
	return r > 0x7F && r > g + b;
}
bool IsRowMarker(Uint8* Pixel, SDL_PixelFormat *fmt) {
	return IsColMarker(Pixel, fmt);
}

bool IsBoxMarker(Uint8* Pixel, SDL_PixelFormat *fmt) {
	int r, g, b;
	Uint32 color= *((Uint32*)Pixel);
	r= (color & fmt->Rmask) >> (fmt->Rshift - fmt->Rloss);
	g= (color & fmt->Gmask) >> (fmt->Gshift - fmt->Gloss);
	b= (color & fmt->Bmask) >> (fmt->Bshift - fmt->Bloss);
	return g > 0x7F && g > r + b;
}

Uint8* NextCtrlCol(Uint8* CtrlRow, Uint8* LastCtrlCol, SDL_Surface *Img) {
	int PxSize= Img->format->BytesPerPixel;
	int Width= Img->w;
	Uint8* LastPixel= CtrlRow+Width*PxSize;
	Uint8* X= LastCtrlCol+PxSize;
	while (CtrlRow-X < Width && !IsColMarker(X, Img->format))
		X+= PxSize;
	return X;
}

Uint8* NextCtrlRow(Uint8* Pixels, Uint8* LastCtrlRow, SDL_Surface *Img) {
	Uint8 *LastPixel= Pixels + Img->h*Img->pitch;
	Uint8 *Y= LastCtrlRow + Img->pitch;
	while (Y<LastPixel && !IsRowMarker(Y, Img->format))
		Y+= Img->pitch;
	return Y;
}

bool FindBoxMarkers(int* Markers, Uint8* Corner, int offset, int step, SDL_PixelFormat *fmt, const char *TargetName) {
	int MarkerIdx= 1;
	Uint8 *Pos;
	// There should be either 2 or 0 marker pixels.  Else is an error.
	for (Pos= Corner+offset; Pos>Corner; Pos-=step) {
		if (IsBoxMarker(Pos, fmt))
			if (MarkerIdx >= 0)
				Markers[MarkerIdx--]= ((Pos-Corner) / step) - 1;
			else {
				fprintf(stderr, "More than two spacing markers found in the control %s.\n", TargetName);
				return false;
			}
	}
	if (MarkerIdx == 0) {
		fprintf(stderr, "Only one spacing marker found in the contorl %s.\n", TargetName);
		return false;
	}
	return true;
}

bool ProcessChar(CharCoordts* CC, SDL_Surface *Img, Uint8 *Corner, int CellPxWidth, int CellPxHeight) {
	// Cell refers to the image, not including control rows
	// Box refers to the portion of the cell which counts as the dimensions of
	//   the character for text-formatting purposes. i.e. BoxY1 is the
	//   baseline for the character.
	int BoxX[2]= { 0, CellPxWidth}, BoxY[2]= { 0, CellPxHeight };
	int BoxWidth, BoxHeight;
	Uint8 *ImgStart= (Uint8*)Img->pixels;
	int PxSize= Img->format->BytesPerPixel;
	int CellY= (Corner - ImgStart)/Img->pitch + 1;
	int CellX= (Corner - (ImgStart+(CellY-1)*Img->pitch))/Img->format->BytesPerPixel + 1;
	DEBUGMSG(("  Char@(%d,%d)%dx%d", CellX, CellY, CellPxWidth, CellPxHeight));
	// Scan backward accross the control row looking for box markers.
	if (!FindBoxMarkers(BoxX, Corner, CellPxWidth*PxSize, PxSize, Img->format, "row"))
		return false;
	// Scan up the control column looking for box markers.
	if (!FindBoxMarkers(BoxY, Corner, CellPxHeight*Img->pitch, Img->pitch, Img->format, "column"))
		return false;
	BoxWidth= BoxX[1]-BoxX[0];
	BoxHeight= BoxY[1]-BoxY[0];
	// texture coordinates.  percentage of overall texture
	CC->X0t= CellX / (double)Img->w;
	CC->X1t= (CellX+CellPxWidth) / (double)Img->w; // XXX should it be (X+Width-1)?
	CC->Y0t= CellY / (double)Img->h;
	CC->Y1t= (CellY+CellPxHeight) / (double)Img->h; // XXX ditto
	CC->X0v= -BoxX[0]; // negative distance from box to reach left edge of cell
	CC->X1v= CellPxWidth-BoxX[0]; // positive distance from left edge of box to right edge of cell
	CC->Y0v= BoxY[1]; // positive distance from bottom of box to top of cell (positive=up)
	CC->Y1v= BoxY[1]-CellPxHeight; // negative distance from bottom of box to bottom of cell
	CC->BoxWidth= BoxWidth;
	CC->BoxHeight= BoxHeight;
	return true;
}

void NormalizeFontSize(Font *F) {
	double TotalHeight= 0.0, Scale;
	int i;
	for (i=0; i<FONT_CHAR_MAX; i++)
		TotalHeight+= F->Chars[i].BoxHeight;
	Scale= FONT_CHAR_MAX / AvgHeight;
	for (i=0; i<FONT_CHAR_MAX; i++) {
		F->Chars[i].BoxHeight*= Scale;
		F->Chars[i].BoxWidth*= Scale;
		F->Chars[i].X0v*= Scale;
		F->Chars[i].X1v*= Scale;
		F->Chars[i].Y0v*= Scale;
		F->Chars[i].Y1v*= Scale;
	}
}

bool MapCharacterCoordts(SDL_Surface* Img, Font *F) {
	Uint8 *ImgStart, *PrevRow, *NextRow, *PrevCol, *NextCol;
	int i, CurChar, ImgColNo, ImgRowNo;
	int CellWidth, CellHeight;

	if (!IsRowMarker(Img->pixels, Img->format)) {
		fprintf(stderr, "All font textures must have a cell-marker pixel in the upper-left corner.\n");
		return false;
	}
	ImgStart= Img->pixels;
	CurChar= 0;
	PrevRow= ImgStart;
	ImgRowNo= 0;
	DEBUGMSG(("Searching for characters..."));
	while (CurChar < FONT_CHAR_MAX) {
		NextRow= NextCtrlRow(ImgStart, PrevRow, Img);
		DEBUGMSG((" row@%d", (NextRow-ImgStart)/Img->pitch));
		CellHeight= (NextRow-PrevRow)/Img->pitch - 1;
		if (CellHeight <= 0) break;
		PrevCol= PrevRow;
		ImgColNo= 0;
		while (CurChar < FONT_CHAR_MAX) {
			NextCol= NextCtrlCol(PrevRow, PrevCol, Img);
			DEBUGMSG((" col@%d", (NextCol-ImgStart)/Img->format->BytesPerPixel));
			CellWidth= (NextCol-PrevCol)/Img->format->BytesPerPixel - 1;
			if (CellWidth <= 0) break;
			if (!ProcessChar(&(F->Chars[CurChar]), Img, PrevCol, CellWidth, CellHeight)) {
				fprintf(stderr, "Invalid character-box markers for '%c' at col %d, row %d of image.\n", (char)(' '+CurChar), ImgColNo, ImgRowNo);
				return false;
			}
			CurChar++;
			PrevCol= NextCol;
			ImgColNo++;
		}
		PrevRow= NextRow;
		ImgRowNo++;
	}
	if (CurChar != FONT_CHAR_MAX) {
		fprintf(stderr, "Font only defined characters 0x20-0x%X. Need 0x20-0x7F.\n", 0x20+CurChar-1);
		return false;
	}
	NormalizeFontSize(F);
	return true;
}

bool BuildFontTexture(SDL_Surface* Img, Font *Result) {
	glBindTexture(GL_TEXTURE_2D, Result->Texture);
	LoadImgIntoTexture(Img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void BuildDisplayLists(SDL_Surface* Img, Font *Result) {
	int i;
	CharCoordts *CC;

	for (i=0; i<FONT_CHAR_MAX; i++) {
		glNewList(Result->DisplayList0+i, GL_COMPILE);
		CC= &(Result->Chars[i]);
		glBegin(GL_QUADS);
		glTexCoord2d(CC->X0t, CC->Y1t); glVertex3f(CC->X0v, CC->Y0v, 0);
		glTexCoord2d(CC->X1t, CC->Y1t); glVertex3f(CC->X1v, CC->Y0v, 0);
		glTexCoord2d(CC->X1t, CC->Y0t); glVertex3f(CC->X1v, CC->Y1v, 0);
		glTexCoord2d(CC->X0t, CC->Y0t); glVertex3f(CC->X0v, CC->Y1v, 0);
		glEnd();
		glTranslatef(CC->BoxWidth, 0, 0);
		glEndList();
	}
}

Font* Font_Initialize(void* this) {
	Font *Result= (Font*) this;
	glGenTextures(1, &Result->Texture);
	Result->DisplayList0= glGenLists(FONT_CHAR_MAX);
	return Result;
}

void* Font_Finalize(Font* this) {
	glDeleteTextures(1, &this->Texture);
	glDeleteLists(this->DisplayList0, FONT_CHAR_MAX);
	return this;
}

bool GenerateFont(SDL_Surface *Img, Font *F) {
	bool Success;
	SDL_LockSurface(Img);
	Success= MapCharacterCoordts(Img, F);
	if (Success) {
		BuildFontTexture(Img, F);
		BuildDisplayLists(Img, F);
	}
	else {
		fprintf(stderr, "Failed to locate all %d characters in the image.\n", FONT_CHAR_MAX);
	}
	SDL_UnlockSurface(Img);
	return Success;
}

void WriteText(Font *F, const char *Text) {
	GLuint PrevTexture;
	const unsigned char *ch;
	GLboolean TexOn;
	glGetBooleanv(GL_TEXTURE_2D, &TexOn);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &PrevTexture);
	glBindTexture(GL_TEXTURE_2D, F->Texture);
	if (!TexOn)
		glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	DEBUGMSG(("Printing \"%s\"\n", Text));
	for (ch= Text; *ch; ch++)
		if (*ch >= 0x20 && *ch <= 0x7F)
			glCallList(F->DisplayList0+(*ch-' '));
	glPopMatrix();
	if (!TexOn)
		glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, PrevTexture);
}

PUBLISHED(cglText,DoText) {
	ScanParamsResult Result;
	Font *F;
	if (argc < 2) return ERR_PARAMCOUNT;
	if (!ScanParams("FN", argv, &Result)) return ERR_PARAMPARSE;
	if (!Result.Symbolics[0]) return ERR_PARAMPARSE;
	F= Result.Symbolics[0]->Data;
	WriteText(F, Result.FName); // FName was a bad choice.  I should have called it "String"
	return 0;
}

