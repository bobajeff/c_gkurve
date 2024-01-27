#ifndef TRIANGULATE_POLYGON_
#define TRIANGULATE_POLYGON_
#include <cglm/cglm.h>
#include "memory_alloc_helpers.h"

int triangulatePolygon(Vec2Buffer polygon, IntBuffer * triangles);

#endif // TRIANGULATE_POLYGON_