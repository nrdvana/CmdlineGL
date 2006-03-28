#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

SDL_Surface *LoadImg(const char *filename);
void LoadImgIntoTexture(SDL_Surface *Img);

PUBLISHED(cglLoadImage2D, DoLoadImage2D);

#endif
