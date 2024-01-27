/*
    Note: this is just a quick and dirty implementation which is why it only supports ASCII characters.
    I intend to abandon this immediately as it only produces unscalable text. So it merely serves as practice 
    for using the freetype API.
*/
#include "label.h"
#include "atlas.h"
#include "cglm/types.h"
#include "cglm/vec2.h"
#include "draw.h"
#include "freetype/ftimage.h"
#include <assert.h>
#include <ft2build.h>
#include <stdio.h>
#include <stdlib.h>
#include FT_FREETYPE_H

#define TOTAL_ASCII_CHARS 128

typedef struct RGBA8_Pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} RGBA8_Pixel;

GlyphInfo * labelInit(char * font_path, int char_size, ImageData **imgs, size_t * num_imgs){
    FT_Library library;
    FT_Error error;
    error = FT_Init_FreeType(&library);
    if (error){
        // handle error
    }
    FT_Face face;
    FT_Long face_index = 0;
    FT_New_Face(library, font_path, face_index, &face);

    unsigned int i;
    // Get all ascii characters from font
    ImageData font_imgs[TOTAL_ASCII_CHARS];
    GlyphInfo * glyph_info = malloc(sizeof(GlyphInfo) * TOTAL_ASCII_CHARS);

    for (i = 0; i < TOTAL_ASCII_CHARS; i++){
        //get glyph info from char_map
        char char_code = i;
        FT_Set_Char_Size(face, char_size * 64, 0, 50, 0);
        FT_Load_Char(face, char_code, FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;
        //get glyph bitmap data
        FT_Bitmap glyph_bitmap = glyph->bitmap;
        unsigned int glyph_width = glyph_bitmap.width;
        unsigned int glyph_height = glyph_bitmap.rows;
        unsigned int pad_width = glyph_width + 2;
        unsigned int pad_height = glyph_height + 2;
        unsigned char * glyph_buffer = glyph_bitmap.buffer;
        unsigned int pixel_y, pixel_x;
        // Add 1 pixel padding to texture to avoid bleeding over other textures
        RGBA8_Pixel * rgba_buffer = malloc(pad_width * pad_height * 4);
        for (pixel_y = 0; pixel_y < pad_height; pixel_y++){
            for (pixel_x = 0; pixel_x < pad_width; pixel_x++){
                unsigned int pixel_index = (pixel_y * pad_width) + pixel_x;
                unsigned char pixel_value = 0;
                if (!(pixel_x == 0 || pixel_x == (glyph_width + 1) || pixel_y == 0 || pixel_y == (glyph_height + 1))){
                    unsigned int glyph_buffer_index = ((pixel_y - 1) * glyph_width) + (pixel_x - 1);
                    pixel_value = glyph_buffer[glyph_buffer_index];
                };
                rgba_buffer[pixel_index].r = pixel_value;
                rgba_buffer[pixel_index].g = pixel_value;
                rgba_buffer[pixel_index].b = pixel_value;
                rgba_buffer[pixel_index].a = pixel_value;
            }
        }
        font_imgs[i] = (ImageData){.width = pad_width, .height = pad_height, .data = (unsigned char *)rgba_buffer, .padded = true};
        glyph_info[i] = (GlyphInfo){.image_data_index = i + *num_imgs, .metrics = glyph->metrics};
    }
    int n = *num_imgs;

    *imgs = realloc(*imgs, sizeof(ImageData) * (*num_imgs + TOTAL_ASCII_CHARS));
    ImageData * imgs_ = *imgs;
    memcpy(&imgs_[*num_imgs], font_imgs, sizeof(ImageData) * TOTAL_ASCII_CHARS);
    *num_imgs += TOTAL_ASCII_CHARS;
    return glyph_info;
}

void drawLabel(App * app, GlyphInfo * glyph_info, ImageData *imgs, char * ascii_string, vec2 position, vec4 text_color){
    int i;
    vec2 offset = {0.0,0.0};
    unsigned long string_length = strlen(ascii_string);
    for (i = 0; i < string_length; i++){
        unsigned char char_code = ascii_string[i];
        assert(char_code < TOTAL_ASCII_CHARS); //This interface only support ASCII characters for simplicity

        FT_Glyph_Metrics metrics = glyph_info[char_code].metrics;
        if (char_code == 10){ //10 is newline character (\n)
            offset[0] = 0.0;
            offset[1] -= (float)(metrics.height >> 6);
        } else if (char_code == ' '){
            offset[0] += metrics.horiAdvance >> 6;
        } else {
            vec2 metrics_pos = {(float)(metrics.horiBearingX >> 6), (float)((metrics.horiBearingY - metrics.height) >> 6)};
            vec2 scale = {(float)(metrics.width >> 6), (float)(metrics.height >> 6)};
            vec2 pos_plus_offset;
            vec2 draw_pos;
            glm_vec2_add(position, offset, pos_plus_offset);
            glm_vec2_add(pos_plus_offset, metrics_pos, draw_pos);
            drawQuad(
                app, draw_pos, scale,
                (FragUniform){.type = GkurveType_Triangle,
                              .blend_color = {text_color[0], text_color[1],
                                              text_color[2], text_color[3]}},
                imgs[glyph_info[char_code].image_data_index].uv);
            offset[0] += metrics.horiAdvance >> 6;
        }
    }
}