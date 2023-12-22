#ifndef ATLAS_H_
#define ATLAS_H_
#include <cglm/cglm.h>

#include <stddef.h>
#include <sys/types.h>

typedef struct UVData {
  vec2 bottom_left;
  vec2 width_and_height;
} UVData;

typedef struct Atlas {
  unsigned char * texture_data;
  size_t texture_data_size;
  size_t texture_side_length;
} Atlas;

typedef struct ImageData {
  int width;
  int height;
  unsigned char * data;
  UVData uv_data;
} ImageData;

Atlas atlasCreate(ImageData * imgs, size_t num_imgs, size_t texture_side_length);

void atlasDestroy(Atlas atlas);

#endif // ATLAS_H_