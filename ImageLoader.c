#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include "Global.h"
#include "ImageLoader.h"


// Following code taken and modified slightly from nehe.gamedev.net
// --------------------------------------------------------------------
//
// This code was created by Jeff Molofee '99 (ported to Linux/GLUT by Richard Campbell '99)
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe 
// (email Richard Campbell at ulmont@bellsouth.net)
//

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
bool LoadImage(const char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
//    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
		fprintf(stderr, "File Not Found : %s\n",filename);
		return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->Width, 4, 1, file)) != 1) {
		fprintf(stderr, "Error reading width from %s.\n", filename);
		return 0;
    }
    fprintf(stderr, "Width of %s: %lu\n", filename, image->Width);
    
    // read the height 
    if ((i = fread(&image->Height, 4, 1, file)) != 1) {
		fprintf(stderr, "Error reading height from %s.\n", filename);
		return 0;
    }
    fprintf(stderr, "Height of %s: %lu\n", filename, image->Height);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->Width * image->Height * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
		fprintf(stderr, "Error reading planes from %s.\n", filename);
		return 0;
    }
    if (planes != 1) {
		fprintf(stderr, "Planes from %s is not 1: %u\n", filename, planes);
		return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
		fprintf(stderr, "Error reading bpp from %s.\n", filename);
		return 0;
    }
    if (bpp != 24) {
		fprintf(stderr, "Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->Data= malloc(size);
    if (image->Data == NULL) {
		fprintf(stderr, "Error allocating memory for color-corrected image data");
		return 0;
    }

    if ((i = fread(image->Data, size, 1, file)) != 1) {
		fprintf(stderr, "Error reading image data from %s.\n", filename);
		return 0;
    }

//    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
//		temp = image->Data[i];
//		image->data[i] = image->Data[i+2];
//		image->data[i+2] = temp;
//    }
    
    // we're done.
    return 1;
}
