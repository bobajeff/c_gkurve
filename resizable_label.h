#ifndef RESIZABLE_LABEL_
#define RESIZABLE_LABEL_
#include <cglm/cglm.h>
#include "atlas.h"
#include "draw.h"
#include "freetype/freetype.h"
#include "freetype/ftoutln.h"
#include "memory_alloc_helpers.h"

typedef struct CharVertices {
    VertexBuffer filled_vertices;
    U16Buffer filled_vertices_indices;
    // Concave vertices belong to the filled_vertices list, so just index them
    U16Buffer concave_vertices;
    // The point outside of the convex bezier, doesn't belong to the filled vertices,
    // But the other two points do, so put those in the indices
    VertexBuffer convex_vertices;
    U16Buffer convex_vertices_indices;
} CharVertices;

typedef struct OutlineContext {
    /// There may be more than one polygon (for example with 'i' we have the polygon of the base and
    /// another for the circle)
    Vec2BufferBuffer outline_verts;

    /// The internal outline, used for carving the shape. For example in 'a', we would first get the
    /// outline of the entire 'a', but if we stopped there, the center hole would be filled, so we
    /// need another outline for carving the filled polygon.
    Vec2Buffer inside_verts;

    /// For the concave (inner 'o') and convex (outer 'o') beziers
    Vec2Buffer concave_vertices;
    Vec2Buffer convex_vertices;
} OutlineContext;

// Everything needed draw characters in a font
typedef struct ResizableFontData {
    CharVertices * char_vertices; // needed for turning fonts into gkurve triangles
    FT_Glyph_Metrics * metrics; // needed for getting character placement offsets
} ResizableFontData;

ResizableFontData resizableFontDataGenerate(char *font_path);
void drawResizableLabel(App * app, ResizableFontData rfdata, char * ascii_string, vec4 position, vec4 text_color, u_int32_t text_size, AtlasUV white_texture);
void charVerticesGenerateFromOutlineContext(FT_Orientation orientation, OutlineContext outline_ctx, CharVertices * char_vertices);
void drawCharVertices(App * app, CharVertices * char_vertices, vec4 offset, vec4 position, vec4 text_color, u_int32_t text_size, AtlasUV white_texture);


OutlineContext outlineContextInit();
void outlineContextDestroy(OutlineContext ctx);
CharVertices charVerticesInit();
void charVerticesDestroy(CharVertices char_vertices);
void resizableFontDataDestroy(ResizableFontData rfdata);

void uniteOutsideAndInsideVertices(OutlineContext * ctx);
int moveToFunction(const FT_Vector*  _to,  void* user);
int lineToFunction(const FT_Vector*  _to,  void* user);
int conicToFunction(const FT_Vector*  _control, const FT_Vector*  _to,  void* user);
int cubicToFunction(const FT_Vector*  control1, const FT_Vector*  control2, const FT_Vector*  to,  void* user);

void intersectLineSegments(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 ** dest);
bool intersectRayToLineSegment(vec2 ray_origin, vec2 ray_direction, vec2 p1, vec2 p2);
bool pointInPolygon(vec2 p, Vec2BufferBuffer polygon);
#endif // RESIZABLE_LABEL_