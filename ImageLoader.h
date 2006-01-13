#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

typedef struct Image_t {
    unsigned long Width;
    unsigned long Height;
    void *Data;
} Image;

bool LoadImg(const char *filename, Image *image);

#endif
