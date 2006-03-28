#include <SDL.h>
#include "Global.h"
#include "ParseGL.h"
#include "ImageLoader.h"

#ifdef WITH_SDL_IMG_LIB
#include <SDL_image.h>
#endif

// a separate function so that not so much stack gets allocated on calls to LoadImg
void ReportImgNotFound(const char *FName) {
	char buffer[1024];
	getcwd(buffer, sizeof(buffer)-1);
	buffer[sizeof(buffer)-1]= '\0';
	fprintf(stderr, "Error loading image: '%s'\nWorking Dir = '%s'\n", FName, buffer);
}

bool UsableByGL(SDL_Surface *Img) {
	int dim;
	// images must be square
	if (Img->w != Img->h) {
		fprintf(stderr, "OpenGL requires square images. (image is %dx%d)\n", (int)Img->w, (int)Img->h);
		return false;
	}
	// image dimensions must be a power of 2
	for (dim= Img->w; dim != 1; dim>>=1)
		if (dim&1) {
			fprintf(stderr, "OpenGL requires image dimensions to be a power of 2. (%d is not)\n", (int)Img->w);
			return false;
		}
	if (Img->format->BytesPerPixel < 2 || Img->format->BytesPerPixel > 4) {
		fprintf(stderr, "Image pixel format not handled by CmdlineGL. Expecting 16/24/32bpp\n");
		return false;
	}
	// we need a contiguous byte array of pixels
	if (Img->pitch != Img->w * Img->format->BytesPerPixel) {
		fprintf(stderr, "SDL did not load the pixels as a contiguous array of bytes.\n");
		return false;
	}
	return true;
}

SDL_Surface* LoadImg(const char *FName) {
	SDL_Surface *Img;
#ifdef WITH_SDL_IMG_LIB
	Img= IMG_Load(FName);
#else
	Img= SDL_LoadBMP(FName);
#endif
	if (!Img)
		ReportImgNotFound(FName);
	else if (!UsableByGL(Img)) {
		fprintf(stderr, "Unable to use image %s\n", FName);
		SDL_FreeSurface(Img);
		Img= NULL;
	}
	else
		DEBUGMSG(("Loaded image '%s', %dx%d %dbpp\n", FName, Img->w, Img->h, Img->format->BitsPerPixel));
	return Img;
}

// This code seems dirty... but I don't have enough experience with SDL & OpenGL
// on both architecture endiannesses to optimize it.
void LoadImgIntoTexture(SDL_Surface *Img) {
	int format, type;
	SDL_PixelFormat *fmt= Img->format;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	if (fmt->Rmask == 0xFF000000 && fmt->Gmask == 0x00FF0000 && fmt->Bmask == 0x0000FF00) {
	#else
	if (fmt->Rmask == 0x000000FF && fmt->Gmask == 0x0000FF00 && fmt->Bmask == 0x00FF0000) {
	#endif
		format= fmt->Amask? GL_RGBA : GL_RGB;
		type= GL_UNSIGNED_BYTE;
		DEBUGMSG(("Image seems to be %s/GL_UNSIGNED_BYTE\n", format==GL_RGBA?"GL_RGBA":"GL_RGB"));
	}
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	else if (fmt->Bmask == 0xFF000000 && fmt->Gmask == 0x00FF0000 && fmt->Rmask == 0x0000FF00) {
	#else
	else if (fmt->Bmask == 0x000000FF && fmt->Gmask == 0x0000FF00 && fmt->Rmask == 0x00FF0000) {
	#endif
		format= fmt->Amask? GL_BGRA : GL_BGR;
		type= GL_UNSIGNED_BYTE;
		DEBUGMSG(("Image seems to be %s/GL_UNSIGNED_BYTE\n", format==GL_BGRA?"GL_BGRA":"GL_BGR"));
	}
	else {
		// I don't want to implement the rest until I actually have some way to test it.
		// Really, I haven't even tested anything other than BGR.
		fprintf(stderr, "Loading 16-bit images not yet supported.\n");
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Img->w, Img->h, 0, format, type, Img->pixels);
}

PUBLISHED(cglLoadImage2D, DoLoadImage2D) {
	int i, j;
	SDL_Surface *Img;
	ScanParamsResult ScanResult;
	if (argc < 1) return ERR_PARAMCOUNT;
	if (!ScanParams("N", argv, &ScanResult)) return ERR_PARAMPARSE;
	// Now load the image
	Img= LoadImg(ScanResult.FName);
	if (!Img) return ERR_EXEC;
	// Then, load the image data into OpenGL
	SDL_LockSurface(Img);
	LoadImgIntoTexture(Img);
	SDL_UnlockSurface(Img);
	SDL_FreeSurface(Img);
	return 0;
}

