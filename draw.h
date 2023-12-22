#ifndef DRAW_H_
#define DRAW_H_
#include <cglm/cglm.h>
#include <stddef.h>
#include <sys/types.h>
#include "webgpu.h"
#include "atlas.h"
typedef struct Vertex {
  vec4 pos;
  vec2 uv;
} Vertex;

// The uniform read by the vertex shader, it contains the matrix
// that will move vertices
typedef struct VertexUniform {
    mat4 mat;
} VertexUniform;

typedef enum GkurveType {
    GkurveType_Concave = 0u,
    GkurveType_Convex = 1u,
    GkurveType_Filled = 2u,
} GkurveType;

typedef struct FragUniform {
    GkurveType type;
    vec4 blend_color;
    // Padding for struct alignment to 16 bytes (minimum in WebGPU uniform).
    vec3 padding;
} FragUniform;

typedef struct App {
  Vertex * vertices;
  size_t vertices_size;
  FragUniform * fragment_uniform_list;
  size_t fragment_uniform_list_size;
} App;

void drawEquilateralTriangle(App * app, vec2 position, float scale, FragUniform uniform, UVData uv_data);

void drawQuad(App * app, vec2 position, vec2 scale, FragUniform uniform, UVData uv_data);

void drawCircle(App * app, vec2 position, float radius, vec4 blend_color, UVData uv_data);
#endif // DRAW_H_