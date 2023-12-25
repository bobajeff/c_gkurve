#ifndef LABEL_H_
#define LABEL_H_
#include "atlas.h"
#include "draw.h"
#include "freetype/freetype.h"

typedef struct GlyphInfo {
    unsigned int image_data_index;
    FT_Glyph_Metrics metrics;
} GlyphInfo;

GlyphInfo * labelInit(char * font_path, int char_size, ImageData **imgs, size_t * num_imgs);
void drawLabel(App * app, GlyphInfo * glyph_info, ImageData *imgs, char * ascii_string, vec2 position, vec4 text_color);

#endif // LABEL_H_