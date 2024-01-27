#include "resizable_label.h"
#include "atlas.h"
#include "draw.h"
#include <assert.h>
#include <float.h>
#include <ft2build.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include FT_FREETYPE_H
#include "freetype/ftoutln.h"
#include <cglm/cglm.h>
#include <limits.h>
#include <stdlib.h>
#include "triangulate_polygon.h"

#define TOTAL_ASCII_CHARS 128

// #define DEBUG_RESIZEBABLE_LABLE

ResizableFontData resizableFontDataGenerate(char *font_path){
     FT_Library library;
     FT_Error error;
     error = FT_Init_FreeType(&library);
     if (error) {
        // handle error
     }
     FT_Face face;
     FT_Long face_index = 0;
     FT_New_Face(library, font_path, face_index, &face);
     FT_Fixed multiplier = 1024 << 6;
     FT_Matrix matrix = {
         .xx = 1 * multiplier,
         .xy = 0 * multiplier,
         .yx = 0 * multiplier,
         .yy = 1 * multiplier,
     };
     FT_GlyphSlot glyph = face->glyph;
     FT_Set_Transform(face, &matrix, glyph->outline.points);
     uint i;
     CharVertices * char_vertices = malloc(sizeof(CharVertices) * TOTAL_ASCII_CHARS);
     memset(char_vertices, 0, sizeof(CharVertices) * TOTAL_ASCII_CHARS);
     FT_Glyph_Metrics * metrics = malloc(sizeof(FT_Glyph_Metrics) * TOTAL_ASCII_CHARS);
     for (i = 0; i < TOTAL_ASCII_CHARS; i++){
        char char_code = i;
        FT_UInt char_index = FT_Get_Char_Index(face, char_code);
        if (char_index != 0) {
          FT_Load_Char(face, char_code, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
          FT_GlyphSlot glyph = face->glyph;
          FT_Outline ol = glyph->outline;

          FT_Orientation orientation = FT_Outline_Get_Orientation(&ol);
          // Use a big scale and then scale to the actual text size

          FT_Outline_Funcs callbacks = {
              .move_to = moveToFunction,
              .line_to = lineToFunction,
              .conic_to = conicToFunction,
              .cubic_to = cubicToFunction,
              .shift = 0,
              .delta = 0,
          };
          OutlineContext outline_ctx = outlineContextInit();

          FT_Outline_Decompose(&glyph->outline, &callbacks, &outline_ctx);

          uniteOutsideAndInsideVertices(&outline_ctx);

          charVerticesGenerateFromOutlineContext(orientation, outline_ctx,
                                                 &char_vertices[i]);
          outlineContextDestroy(outline_ctx);
         metrics[i] = glyph->metrics;
        }
     }
     ResizableFontData rfdata = {
        .metrics = metrics,
        .char_vertices = char_vertices
     };
    return rfdata;
}

void charVerticesGenerateFromOutlineContext(FT_Orientation orientation, OutlineContext outline_ctx, CharVertices * char_vertices) {

     // call triangulatePolygon() for each polygon, and put the results all
     // in all_outlines and all_indices
     Vec2Buffer all_outlines = vec2BufferInit();
     U16Buffer all_indices = u16BufferInit();
     unsigned int idx_offset = 0;

     unsigned int outline_index;
     for (outline_index = 0; outline_index < outline_ctx.outline_verts.length; outline_index++){
        Vec2Buffer outline = outline_ctx.outline_verts.items[outline_index];
        Vec2Buffer polygon = vec2BufferInit();

        if (outline.length == 0) continue;

        if (orientation == FT_ORIENTATION_TRUETYPE){
            // TrueType orientation has clockwise contours, so reverse the list
            // since we need CCW.
            int i = outline.length - 1;
            while (i > 0) {
                vec2BufferAppend(&polygon, &outline.items[i]);
                i--;
            }
        } else {
            vec2BufferAppendArray(&polygon, outline.items, outline.length);
        }

        IntBuffer triangles = intBufferInit();
        triangulatePolygon(polygon, &triangles);
        
        int i;
        int idx;
        u_int16_t idx_mod_value;
        for (i = 0; i < triangles.length; i++){
            idx = triangles.items[i];
            idx_mod_value = (u_int16_t)(idx) + idx_offset;
            vec2BufferAppend(&all_outlines, &polygon.items[idx]);
            u16BufferAppend(&all_indices, &idx_mod_value);
        }
        idx_offset += triangles.length;
        
        // cleanup
        intBufferDestroy(&triangles);
        vec2BufferDestroy(&polygon);
     }

    CharVertices char_vertices_ = charVerticesInit();
    int idx;

    // set filled_vertices and filled_vertices_indices
    for (idx = 0; idx < all_outlines.length; idx++){
        vec2 item = {all_outlines.items[idx][0], all_outlines.items[idx][1]};
        Vertex filled_vertex = filled_vertex = (Vertex){.pos = {item[0], item[1], 0, 1}};
        glm_vec2_div(item, (vec2){1024 << 6, 1024 << 6}, filled_vertex.uv);
        vertexBufferAppend(&char_vertices_.filled_vertices, &filled_vertex);

    }
    u16BufferAppendArray(&char_vertices_.filled_vertices_indices, all_indices.items, all_indices.length);

    // set concave_vertices (really indexes)
    for (idx = 0; idx < outline_ctx.concave_vertices.length; idx++){
        u_int16_t ol_idx;
        vec2 * concave_control = &outline_ctx.concave_vertices.items[idx];
        for (ol_idx = 0; ol_idx < all_outlines.length; ol_idx++){
                vec2 * item = &all_outlines.items[ol_idx];
                if(glm_vec2_eqv(*item, *concave_control)){
                    u16BufferAppend(&char_vertices_.concave_vertices, &ol_idx);
                    break;
                };
        }
    }
    
    // set convex_vertices and convex_vertices_indices
    assert((outline_ctx.convex_vertices.length % 3) == 0);
    idx = 0;
    while (idx < outline_ctx.convex_vertices.length){
        vec2 vert = {outline_ctx.convex_vertices.items[idx][0], outline_ctx.convex_vertices.items[idx][1]};
        Vertex convex_vertex = {.pos = {vert[0], vert[1], 0, 1}};
        glm_vec2_div(vert, (vec2){1024 << 6, 1024 << 6}, convex_vertex.uv);
        vertexBufferAppendArray(&char_vertices_.convex_vertices, &convex_vertex, 1);
        u_int16_t ol_idx;
        bool found1 = false;
        bool found2 = false;
        for (ol_idx = 0; ol_idx < all_outlines.length; ol_idx++){
            vec2 item = {all_outlines.items[ol_idx][0], all_outlines.items[ol_idx][1]};
            if(found1 == false && glm_vec2_eqv(item, outline_ctx.convex_vertices.items[idx + 1])){
                u16BufferAppend(&char_vertices_.convex_vertices_indices, &ol_idx);
                found1 = true;
            }
            else if(found2 == false && glm_vec2_eqv(item, outline_ctx.convex_vertices.items[idx + 2])){
                u16BufferAppend(&char_vertices_.convex_vertices_indices, &ol_idx);
                found2 = true;
            }

            if (found1 && found2){
                found1 = false;
                found2 = false;
                break;
            }
        }
        idx += 3;
    }
    assert(((char_vertices_.convex_vertices.length + char_vertices_.convex_vertices_indices.length) % 3) == 0);

    // Cleanup
    vec2BufferDestroy(&all_outlines);
    u16BufferDestroy(&all_indices);

    *char_vertices = char_vertices_;
}

void drawResizableLabel(App * app, ResizableFontData rfdata, char * ascii_string, vec4 position, vec4 text_color, u_int32_t text_size, AtlasUV white_texture){
    CharVertices * char_vertices = rfdata.char_vertices;
    FT_Glyph_Metrics * metrics = rfdata.metrics;
    vec4 offset = {0.0,0.0, 0.0, 0.0};
    unsigned long string_length = strlen(ascii_string);
    int i;
    for (i = 0; i < string_length; i++){
        unsigned char char_code = ascii_string[i];
        if (char_code == 10){ //10 is newline character (\n)
            offset[0] = 0.0;
            offset[1] -= (float)(metrics['I'].vertAdvance * (float)((float)text_size / 1024)); // Just use 'I' here to get metrics data since '\n' doesn't have any
        } else if (char_code == ' '){
            offset[0] += (float)(metrics[char_code].horiAdvance * (float)((float)text_size / 1024));
        } else {
            drawCharVertices(app, &char_vertices[char_code], offset, position, text_color, text_size, white_texture);
            offset[0] += (float)(metrics[char_code].horiAdvance * (float)((float)text_size / 1024));
        }
    }
}

void drawCharVertices(App * app, CharVertices * char_vertices, vec4 offset, vec4 position, vec4 text_color, u_int32_t text_size, AtlasUV white_texture) {

    // Read the data and apply resizing of pos and uv
    VertexBuffer filled_vertices_after_offset = vertexBufferInit();
    int i;
    for (i = 0; i < char_vertices->filled_vertices.length; i++){
        Vertex vert = char_vertices->filled_vertices.items[i];
        glm_vec4_mul(vert.pos, (vec4){(float)text_size / 1024, (float)text_size / 1024, 0.0, 1.0}, vert.pos);
        vec4 pos_plus_offset;
        glm_vec4_add(position, offset, pos_plus_offset);
        glm_vec4_add(vert.pos, pos_plus_offset, vert.pos);
        vert.uv[0] = vert.uv[0] * white_texture.width + white_texture.x;
        vert.uv[1] = 1.0 - (vert.uv[1] * white_texture.height + white_texture.y);

        vertexBufferAppendArray(&filled_vertices_after_offset, &vert, 1);
    }
    vertexBufferAppendArray(&app->vertices, filled_vertices_after_offset.items, filled_vertices_after_offset.length);
    size_t fragment_uniforms_length = filled_vertices_after_offset.length / 3;
    for (i = 0; i < fragment_uniforms_length; i++){
        FragUniform fragment_uniform = {};
    #ifdef DEBUG_RESIZEBABLE_LABLE
        glm_vec4_copy((vec4){ 0, 1, 0, 1 }, fragment_uniform.blend_color);
    #else
        glm_vec4_copy(text_color, fragment_uniform.blend_color);
    #endif
        fragment_uniform.type = GkurveType_Triangle;
        fragUniformBufferAppend(&app->fragment_uniform_list, &fragment_uniform);
    }

    int convex_vertices_after_offset_length = char_vertices->convex_vertices.length + char_vertices->convex_vertices_indices.length;
    uint j = 0;
    uint k = 0;
    size_t convex_vertices_consumed = 0;
    while (j < convex_vertices_after_offset_length){
        Vertex val1 = char_vertices->convex_vertices.items[j / 3];
        convex_vertices_consumed++;
        glm_vec4_mul(val1.pos, (vec4){(float)text_size / 1024, (float)text_size / 1024, 0.0, 1.0}, val1.pos);
        vec4 pos_plus_offset;
        glm_vec4_add(position, offset, pos_plus_offset);
        glm_vec4_add(val1.pos, pos_plus_offset, val1.pos);

        val1.uv[0] = val1.uv[0] * white_texture.width + white_texture.x;
        val1.uv[1] = val1.uv[1] * white_texture.height + white_texture.y;

        vertexBufferAppend(&app->vertices, &val1);
        Vertex val2 = filled_vertices_after_offset.items[char_vertices->convex_vertices_indices.items[k]];
        vertexBufferAppend(&app->vertices, &val2);
        Vertex val3 = filled_vertices_after_offset.items[char_vertices->convex_vertices_indices.items[k + 1]];
        vertexBufferAppend(&app->vertices, &val3);
        j += 3;
        k += 2;
    }
    assert(convex_vertices_consumed == char_vertices->convex_vertices.length);
    fragment_uniforms_length = convex_vertices_after_offset_length / 3;
    for (i = 0; i < fragment_uniforms_length; i++){
        FragUniform fragment_uniform = {};
    #ifdef DEBUG_RESIZEBABLE_LABLE
        glm_vec4_copy((vec4){ 1, 0, 0, 1 }, fragment_uniform.blend_color);
    #else
        glm_vec4_copy(text_color, fragment_uniform.blend_color);
    #endif
        fragment_uniform.type = GkurveType_QuadraticConvex;
        fragUniformBufferAppend(&app->fragment_uniform_list, &fragment_uniform);
    }

    int concave_vertices_after_offset_length = char_vertices->concave_vertices.length;
    for (i = 0; i < concave_vertices_after_offset_length; i++){
        vertexBufferAppend(&app->vertices, &filled_vertices_after_offset.items[char_vertices->concave_vertices.items[i]]);
    }
    fragment_uniforms_length = concave_vertices_after_offset_length / 3;
    for (i = 0; i < fragment_uniforms_length; i++){
        FragUniform fragment_uniform = {};
    #ifdef DEBUG_RESIZEBABLE_LABLE
        glm_vec4_copy((vec4){ 0, 0, 1, 1}, fragment_uniform.blend_color);
    #else
        glm_vec4_copy(text_color, fragment_uniform.blend_color);
    #endif
        fragment_uniform.type = GkurveType_QuadraticConcave;
        fragUniformBufferAppend(&app->fragment_uniform_list, &fragment_uniform);
    }
    // cleanup
    vertexBufferDestory(&filled_vertices_after_offset);
}

OutlineContext outlineContextInit(){
    OutlineContext ctx = {
        .outline_verts = vec2BufferBufferInit(),
        .inside_verts = vec2BufferInit(),
        .concave_vertices = vec2BufferInit(),
        .convex_vertices = vec2BufferInit(),
    };
    return ctx;
}

void outlineContextDestroy(OutlineContext ctx){
    vec2BufferBufferDestroy(&ctx.outline_verts);
    vec2BufferDestroy(&ctx.convex_vertices);
    vec2BufferDestroy(&ctx.inside_verts);
    vec2BufferDestroy(&ctx.concave_vertices);
}

CharVertices charVerticesInit(){
    CharVertices char_vertices = {
        .filled_vertices = vertexBufferInit(),
        .filled_vertices_indices = u16BufferInit(),
        .concave_vertices = u16BufferInit(),
        .convex_vertices = vertexBufferInit(),
        .convex_vertices_indices = u16BufferInit(),
    };
    return char_vertices;
}

void charVerticesDestroy(CharVertices char_vertices){
    vertexBufferDestory(&char_vertices.filled_vertices);
    vertexBufferDestory(&char_vertices.convex_vertices);
    u16BufferDestroy(&char_vertices.filled_vertices_indices);
    u16BufferDestroy(&char_vertices.convex_vertices_indices);
    u16BufferDestroy(&char_vertices.concave_vertices);
}

void resizableFontDataDestroy(ResizableFontData rfdata){
    int i;
    for (i = 0; i < TOTAL_ASCII_CHARS; i++){
            CharVertices char_vertices = rfdata.char_vertices[i];
            charVerticesDestroy(char_vertices);
    }
    free(rfdata.metrics);
}

/// If there are elements in inside_verts, unite them with the outline_verts, effectively carving
/// the shape
void uniteOutsideAndInsideVertices(OutlineContext * ctx){
    if (ctx->inside_verts.length != 0){
        // Check which point of outline is closer to the first of inside
        assert(ctx->outline_verts.length != 0);
        Vec2Buffer * last_outline = &ctx->outline_verts.items[ctx->outline_verts.length - 1];
        if (last_outline->length == 0 && ctx->outline_verts.length >= 2) {
            last_outline = &ctx->outline_verts.items[ctx->outline_verts.length - 2];
        }
        assert(last_outline->length != 0);
        size_t closest_to_inside_index = 0;
        bool closest_to_inside_found = false;
        {
            vec2 first_point_inside = {ctx->inside_verts.items[0][0], ctx->inside_verts.items[0][1]};
            float min = FLT_MAX;
            unsigned int i;
            for (i = 0; i < last_outline->length; i++){
                vec2 difference, product;
                glm_vec2_sub(first_point_inside, last_outline->items[i], difference);
                glm_vec2_mul(difference, difference, product);
                float dist = product[0] + product[1];
                if (dist < min){
                    min = dist;
                    closest_to_inside_index = i;
                    closest_to_inside_found = true;
                }
            }
        }
        assert(closest_to_inside_found == true);

        // append closest_to_inside to inside_verts
        vec2BufferAppend(&ctx->inside_verts, &last_outline->items[closest_to_inside_index]);
        // insert inside_verts into last_outline at insert_index
        vec2BufferInsertArray(last_outline, closest_to_inside_index + 1, ctx->inside_verts.items, ctx->inside_verts.length);
        // clear inside_verts
        vec2BufferClear(&ctx->inside_verts);
    }
}

int moveToFunction(const FT_Vector*  _to,  void* user){
    OutlineContext * ctx = user;
    uniteOutsideAndInsideVertices(ctx);
    
    vec2 to = {_to->x, _to->y};

    // To check wether a point is carving a polygon, use the point-in-polygon test to determine if
    // we're inside or outside of the polygon.
    bool new_point_is_inside = pointInPolygon(to, ctx->outline_verts);

    if (ctx->outline_verts.length == 0 || ctx->outline_verts.items[ctx->outline_verts.length - 1].length > 0) {
        // The last polygon we were building is now finished.
        Vec2Buffer new_outline_list = vec2BufferInit();
        vec2BufferBufferAppend(&ctx->outline_verts, &new_outline_list);
    }

    if (new_point_is_inside){
        vec2BufferAppend(&ctx->inside_verts, &to);
    } else {
        // Otherwise create a new polygon
        vec2BufferAppend(&ctx->outline_verts.items[ctx->outline_verts.length - 1], &to);
    }
    return 0;
};

int lineToFunction(const FT_Vector*  _to,  void* user){
    OutlineContext * ctx = user;
    vec2 to = {_to->x, _to->y};
    // If inside_verts is not empty, we need to fill it
    if (ctx->inside_verts.length != 0) {
        vec2BufferAppend(&ctx->inside_verts, &to);
    } else {
        // Otherwise append the new point to the last polygon
        assert(ctx->outline_verts.length != 0);
        vec2BufferAppend(&ctx->outline_verts.items[ctx->outline_verts.length - 1], &to);
    }

    return 0;
};

/// Called to indicate that a quadratic bezier curve occured between the previous point on the glyph
/// outline to the `_to` point on the path, with the specified `_control` quadratic bezier control
/// point.
int conicToFunction(const FT_Vector*  _control, const FT_Vector*  _to,  void* user){
    // printf("%ld, %ld, %ld, %ld\n\n", _control->x, _control->y, _to->x, _to->y);
    OutlineContext * ctx = user;
    vec2 control = {_control->x, _control->y};
    vec2 to = {_to->x, _to->y};

    // If our last point was inside the glyph (e.g. the hole in the letter 'o') then this is a
    // continuation of that path, and we should write this vertex to inside_verts. Otherwise we're
    // on the outside and the vertex should go in outline_verts.
    //
    // We derive if we're on the inside or outside based on whether inside_verts has items in it,
    // because only a lineTo callback can move us from the inside to the outside or vice-versa. A
    // quadratic bezier would *always* be the continuation of an inside or outside path.
    assert(ctx->outline_verts.length != 0);
    Vec2Buffer * verts_to_write = (ctx->inside_verts.length != 0) ? &ctx->inside_verts : &ctx->outline_verts.items[ctx->outline_verts.length - 1];
    vec2 previous_point = {
        verts_to_write->items[verts_to_write->length - 1][0],
        verts_to_write->items[verts_to_write->length - 1][1],
    };

    vec2 vertices[3] = {
        {control[0], control[1]},
        {to[0], to[1]},
        {previous_point[0], previous_point[1]},
    };

    vec2 v1;
    glm_vec2_sub(control, previous_point, v1);
    vec2 v2;
    glm_vec2_sub(to, control, v2);
    // CCW (convex) or CW (concave)?
    if ((v1[0] * v2[1] - v1[1] * v2[0]) <= 0) {
        // Convex
        vec2BufferAppendArray(&ctx->convex_vertices, vertices, 3);
        vec2BufferAppend(verts_to_write, &to);
        return 0;
    }

    // Concave
    //
    // In this case, we need to write a vertex (for the filled triangle) to the quadratic
    // control point. However, since this is the concave case the control point could be outside
    // the shape itself. We need to ensure it is not, otherwise the triangle would end up filling
    // space outside the shape.
    //
    // Diagram: https://user-images.githubusercontent.com/3173176/189944586-bc1b109a-62c4-4ef5-a605-4c6a7e4a1abd.png
    //
    // To fix this, we must determine if the control point intersects with any of our outline
    // segments. If it does, we use that intersection point as the vertex. Otherwise, it doesn't go
    // past an outline segment and we can use the control point just fine.
    vec2 * intersection = NULL;
    int poly_idx;
    for (poly_idx = 0; poly_idx < ctx->outline_verts.length; poly_idx++) {
        Vec2Buffer polygon = ctx->outline_verts.items[poly_idx];
        size_t i = 1;
        while (i < polygon.length) {
            vec2 v1 = {polygon.items[i - 1][0], polygon.items[i - 1][1]};
            vec2 v2 = {polygon.items[i][0], polygon.items[i][1]};
            if (glm_vec2_eqv(v1, previous_point) || glm_vec2_eqv(v1, control) || glm_vec2_eqv(v1, to) || glm_vec2_eqv(v2, previous_point) || glm_vec2_eqv(v2, control) || glm_vec2_eqv(v2, to)) { i++; continue;}
            intersectLineSegments(v1, v2, previous_point, control, &intersection);
            if (intersection != NULL) break;
            i++;
        }
        if (intersection != NULL) break;
    }

    if (intersection){
        vertices[0][0] = (*intersection)[0] * 0.99;
        vertices[0][1] = (*intersection)[1] * 0.99;
    }
    free(intersection);
    vec2BufferAppendArray(&ctx->concave_vertices, vertices, 3);
    vec2BufferAppend(verts_to_write, &vertices[0]);
    vec2BufferAppend(verts_to_write, &to);

    return 0;
};

int cubicToFunction(const FT_Vector*  control1, const FT_Vector*  control2, const FT_Vector*  to,  void* user){
    OutlineContext * ctx = user;
    printf("TODO: search how to approximate cubic bezier with quadratic ones\n");
    assert(false);

    return 0;
};

/// Intersects the line segments [p0, p1] and [p2, p3], returning the intersection point if any.
void intersectLineSegments(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 ** dest) {
    vec2 s1 = {p1[0] - p0[0], p1[1] - p0[1] };
    vec2 s2 = {p3[0] - p2[0], p3[1] - p2[1] };
    float s = (-s1[1] * (p0[0] - p2[0]) + s1[0] * (p0[1] - p2[1])) / (-s2[0] * s1[1] + s1[0] * s2[1]);
    float t = (s2[0] * (p0[1] - p2[1]) - s2[1] * (p0[0] - p2[0])) / (-s2[0] * s1[1] + s1[0] * s2[1]);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        // Collision
        vec2 * rt = malloc(sizeof(vec2));
        assert(rt != NULL);
        (*rt)[0] =  p0[0] + (t * s1[0]);
        (*rt)[1] =  p0[1] + (t * s1[1]);

        *dest = rt;
    } else {
        *dest = NULL;// No collision
    }
}

bool intersectRayToLineSegment(vec2 ray_origin, vec2 ray_direction, vec2 p1, vec2 p2) {
    vec2 product1, product2;
    vec2 * dest;
    glm_vec2_mul(ray_direction, (vec2){10000000.0, 10000000.0}, product1);
    glm_vec2_mul(ray_origin, ray_direction, product2);
    intersectLineSegments(ray_origin, product2, p1, p2, &dest);
    if (dest){
        free(dest);
        return true;
    } else {
        return false;
    }
}

bool pointInPolygon(vec2 p, Vec2BufferBuffer polygon) {
    // Cast a ray to the right of the point and check
    // when this ray intersects the edges of the polygons,
    // if the number of intersections is odd -> inside,
    // if it's even -> outside
    bool is_inside = false;
    int poly_idx;
    for (poly_idx = 0; poly_idx < polygon.length; poly_idx++) {
        Vec2Buffer contour = polygon.items[poly_idx];
        size_t i = 1;
        while (i < contour.length) {
            vec2 v1 = {contour.items[i - 1][0], contour.items[i - 1][1]};
            vec2 v2 = {contour.items[i][0], contour.items[i][1]};
            vec2 ret;
            if (intersectRayToLineSegment(p, (vec2){ 1, p[1] }, v1, v2)) {
                is_inside = !is_inside;
            }
            i++;
        }
    }
    return is_inside;
}