#include "atlas.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define PIXEL_SIZE 4 //(4 rgba values)
#define NUM_NODES 64

Atlas atlasCreate(ImageData *imgs, size_t num_imgs, size_t texture_side_length) {
  stbrp_context context = {};
  stbrp_node nodes[NUM_NODES];
  // Atlas texture is square
  stbrp_init_target(&context, texture_side_length, texture_side_length, nodes, NUM_NODES);
  stbrp_rect *rects = malloc(sizeof(stbrp_rect) * num_imgs);
  size_t i;
  for (i = 0; i < num_imgs; i++) {
    rects[i] = (stbrp_rect){.id = i,
                                  .w = imgs[i].width,
                                  .h = imgs[i].height,
                                  .x = 0,
                                  .y = 0,
                                  .was_packed = 0};
  };
  stbrp_pack_rects(&context, rects, num_imgs);
  size_t texture_data_size = texture_side_length * texture_side_length * PIXEL_SIZE;
  unsigned char * texture_data = malloc(texture_data_size);
  memset(texture_data, 0, texture_data_size);
  for (i = 0; i < num_imgs; i++) {
    int image_idx = rects[i].id;
    stbrp_coord x = rects[i].x;
    stbrp_coord y = rects[i].y;
    stbrp_coord width = rects[i].w;
    stbrp_coord height = rects[i].h;
    size_t img_y;
    for (img_y = 0; img_y < height; img_y++) {
      u_int32_t tex_offset = ((y + img_y) * (texture_side_length * PIXEL_SIZE)) + (x * PIXEL_SIZE);
      u_int32_t data_offset = img_y * (width * PIXEL_SIZE);
      memcpy(&texture_data[tex_offset],
             &imgs[image_idx].data[data_offset], width * PIXEL_SIZE);
    }
    if (imgs[image_idx].padded){
      stbrp_rect padded_rect = rects[i];
      padded_rect.x += 1;
      padded_rect.y += 1;
      padded_rect.w -= 2;
      padded_rect.h -= 2;
      imgs[image_idx].uv = calculateUV(padded_rect, texture_side_length);
    } else {
      imgs[image_idx].uv = calculateUV(rects[i], texture_side_length);
    }
  };
  free(rects);
  return (Atlas){texture_data, texture_data_size, texture_side_length};
};

void atlasDestroy(Atlas atlas) {
  free(atlas.texture_data);
};

AtlasUV calculateUV(stbrp_rect rect, size_t texture_side_length) {
  AtlasUV uv = {
      .x = (float)rect.x,
      .y = (float)rect.y,
      .width = (float)rect.w,
      .height = (float)rect.h,
  };
  uv.x /= (float)texture_side_length;
  uv.y /= (float)texture_side_length;
  uv.width /= (float)texture_side_length;
  uv.height /= (float)texture_side_length;
  return uv;
};