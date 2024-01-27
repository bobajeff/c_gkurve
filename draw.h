#ifndef DRAW_H_
#define DRAW_H_
#include <cglm/cglm.h>
#include <stddef.h>
#include <sys/types.h>
#include "webgpu.h"
#include "atlas.h"
#include "draw_structs.h"
#include "memory_alloc_helpers.h"


typedef struct App {
  VertexBuffer vertices;
  FragUniformBuffer fragment_uniform_list;
} App;

void drawEquilateralTriangle(App * app, vec2 position, float scale, FragUniform uniform, AtlasUV uv, float height_scale);

void drawQuad(App * app, vec2 position, vec2 scale, FragUniform uniform, AtlasUV uv);

void drawCircle(App * app, vec2 position, float radius, vec4 blend_color, AtlasUV uv);


#endif // DRAW_H_