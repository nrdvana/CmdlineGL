#define INCLUDE_GL
#define INCLUDE_SDL
#include <config.h>
#include "Global.h"
#include "ParseGL.h"
#include "ImageLoader.h"
#include "ProcessInput.h"

bool UsableByGL(SDL_Surface *Img) {
	int dim;
	// images must be square (could get into the ARB_rectangular_texture mess, but then
	// users of CmdlineGL would have to deal with the detection)
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
	//if (Img->pitch != Img->w * Img->format->BytesPerPixel) {
	//	fprintf(stderr, "SDL did not load the pixels as a contiguous array of bytes.\n");
	//	return false;
	//}
	return true;
}

SDL_Surface* LoadImg(const char *FName) {
	SDL_Surface *Img;
#ifdef HAVE_LIBSDL_IMAGE
	Img= IMG_Load(FName);
#else
	Img= SDL_LoadBMP(FName);
#endif
	if (!Img)
		fprintf(stderr, "Error loading image '%s'"
		#ifndef HAVE_LIBSDL_IMAGE
			"; SDL_image not available, so file must be plain bitmap"
		#endif
			, FName);
	else if (!UsableByGL(Img)) {
		fprintf(stderr, "Unable to use image '%s'\n", FName);
		SDL_FreeSurface(Img);
		Img= NULL;
	}
	else
		DEBUGMSG(("Loaded image '%s', %dx%d %dbpp\n", FName, Img->w, Img->h, Img->format->BitsPerPixel));
	return Img;
}

// This code seems dirty... but I don't have enough experience with SDL & OpenGL
// on both architecture endiannesses to optimize it.
bool LoadImgIntoTexture(SDL_Surface *Img) {
	int format, type, internalformat, pitchpix=0, remainder=0;
	SDL_PixelFormat *fmt= Img->format;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	if (fmt->Rmask == 0xFF000000 && fmt->Gmask == 0x00FF0000 && fmt->Bmask == 0x0000FF00) {
	#else
	if (fmt->Rmask == 0x000000FF && fmt->Gmask == 0x0000FF00 && fmt->Bmask == 0x00FF0000) {
	#endif
		format= fmt->Amask? GL_RGBA : GL_RGB;
		internalformat= format;
		type= GL_UNSIGNED_BYTE;
		DEBUGMSG(("Image seems to be %s/GL_UNSIGNED_BYTE\n", format==GL_RGBA?"GL_RGBA":"GL_RGB"));
	}
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	else if (fmt->Bmask == 0xFF000000 && fmt->Gmask == 0x00FF0000 && fmt->Rmask == 0x0000FF00) {
	#else
	else if (fmt->Bmask == 0x000000FF && fmt->Gmask == 0x0000FF00 && fmt->Rmask == 0x00FF0000) {
	#endif
		format= fmt->Amask? GL_BGRA : GL_BGR;
		internalformat= format;
		type= GL_UNSIGNED_BYTE;
		DEBUGMSG(("Image seems to be %s/GL_UNSIGNED_BYTE\n", format==GL_BGRA?"GL_BGRA":"GL_BGR"));
	}
	else {
		// I don't want to implement the rest until I actually have some way to test it.
		// Really, I haven't even tested anything other than BGR.
		fprintf(stderr, "Currently the only supported formats are BGR(A) or RGB(A).\n");
		return false;
	}
	
	// This should probably never happen, since dimensions are powers of two, but just in case...
	if (Img->pitch != Img->w * Img->format->BytesPerPixel) {
		pitchpix= Img->pitch / Img->format->BytesPerPixel;
		remainder= Img->pitch % Img->format->BytesPerPixel;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, pitchpix);
		if (remainder)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1+(remainder|(remainder>>1)|(remainder>>2)));
	}
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, Img->w, Img->h, 0, format, type, Img->pixels);
	// Restore defaults
	if (pitchpix)  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	if (remainder) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	return true;
}

COMMAND(cglLoadImage2D, "/") {
	int success;
	SDL_Surface *Img;

	if (!(Img= LoadImg(parsed->strings[0])))
		return false;
	// Then, load the image data into OpenGL
	SDL_LockSurface(Img);
	success= LoadImgIntoTexture(Img);
	SDL_UnlockSurface(Img);
	SDL_FreeSurface(Img);
	return success;
}

