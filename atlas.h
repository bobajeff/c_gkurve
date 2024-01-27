#ifndef ATLAS_H_
#define ATLAS_H_
#include <cglm/cglm.h>

#include <stddef.h>
#include <sys/types.h>
#include "stb_rect_pack.h"

typedef struct AtlasUV {
  float x;
  float y;
  float width;
  float height;
} AtlasUV;


typedef struct Atlas {
  unsigned char * texture_data;
  size_t texture_data_size;
  size_t texture_side_length;
} Atlas;

typedef struct ImageData {
  int width;
  int height;
  unsigned char * data;
  AtlasUV uv;
  bool padded;
} ImageData;

Atlas atlasCreate(ImageData * imgs, size_t num_imgs, size_t texture_side_length);

void atlasDestroy(Atlas atlas);

AtlasUV calculateUV(stbrp_rect rect, size_t texture_side_length);
#endif // ATLAS_H_