#include <cglm/cglm.h>

#ifndef DRAW_STRUCTS_H_
#define DRAW_STRUCTS_H_

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
    GkurveType_QuadraticConvex = 0u,
    GkurveType_SemicircleConvex = 1u,
    GkurveType_QuadraticConcave = 2u,
    GkurveType_SemicircleConcave = 3u,
    GkurveType_Triangle = 4u,
} GkurveType;

typedef struct FragUniform {
    GkurveType type;
    vec4 blend_color;
    // Padding for struct alignment to 16 bytes (minimum in WebGPU uniform).
    vec3 padding;
} FragUniform;

#endif