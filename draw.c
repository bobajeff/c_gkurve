#include "draw.h"
#include "atlas.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void drawEquilateralTriangle(App * app, vec2 position, float scale, FragUniform uniform, AtlasUV uv, float height_scale){
    float triangle_height = scale * sqrtf(0.75) * height_scale;

    Vertex vertices[] = {
        { 
            .pos = { position[0] + scale / 2, position[1] + triangle_height, 0, 1 }, 
            .uv = {uv.x + uv.width * 0.5, 1.0 - uv.y }
        },
        { 
            .pos = { position[0], position[1], 0, 1 },
            .uv = {uv.x, 1.0 - (uv.y + uv.height)}
            },
        { 
            .pos = { position[0] + scale, position[1], 0, 1 },
            .uv = { uv.x + uv.width, 1.0 - (uv.y + uv.height) }
        },
    };

    // append to app data
    vertexBufferAppendArray(&app->vertices, vertices, 3);
    fragUniformBufferAppend(&app->fragment_uniform_list, &uniform);
};

void drawQuad(App * app, vec2 position, vec2 scale, FragUniform uniform, AtlasUV uv){

    Vertex vertices[] = {
        { .pos = { position[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0], position[1], 0, 1 }},
        { .pos = { position[0] + scale[0], position[1], 0, 1 }},

        { .pos = { position[0] + scale[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0] + scale[0], position[1], 0, 1 }},
    };

    vec2 bottom_left_uv = { uv.x, 1.0 - (uv.y + uv.height) };
    vec2 bottom_right_uv = { uv.x + uv.width, 1.0 - (uv.y + uv.height)};
    vec2 top_left_uv = { uv.x, 1.0 - uv.y };
    vec2 top_right_uv = { uv.x + uv.width, 1.0 - uv.y };

    glm_vec2_copy(top_left_uv, vertices[0].uv);
    glm_vec2_copy(bottom_left_uv, vertices[1].uv);
    glm_vec2_copy(bottom_right_uv, vertices[2].uv);
    glm_vec2_copy(top_right_uv, vertices[3].uv);
    glm_vec2_copy(top_left_uv, vertices[4].uv);
    glm_vec2_copy(bottom_right_uv, vertices[5].uv);
;
    FragUniform fragment_uniforms[] = {uniform, uniform};

    // append to app data
    vertexBufferAppendArray(&app->vertices, vertices, 6);
    fragUniformBufferAppendArray(&app->fragment_uniform_list, fragment_uniforms, 2);
};

void drawCircle(App * app, vec2 position, float radius, vec4 blend_color, AtlasUV uv){
    Vertex low_mid = {
        .pos = { position[0], position[1] - (radius * 2.0), 0, 1 },
        .uv = { uv.x + uv.width * 0.5, 1.0 - (uv.y + uv.height) },
    };
    Vertex high_mid = {
        .pos = { position[0], position[1] + (radius * 2.0), 0, 1 },
        .uv = { uv.x + uv.width * 0.5, 1.0 - uv.y },
    };

    Vertex mid_left = {
        .pos = { position[0] - radius, position[1], 0, 1 },
        .uv = { uv.x, 1.0 - (uv.y + uv.height * 0.5) },
    };
    Vertex mid_right = {
        .pos = { position[0] + radius, position[1], 0, 1 },
        .uv = { uv.x + uv.width, 1.0 - (uv.y + uv.height * 0.5) },
    };

    Vertex vertices[] = {
        high_mid,
        mid_left,
        mid_right,

        low_mid,
        mid_left,
        mid_right
    };
    #define BLEND_COLOR {blend_color[0], blend_color[1], blend_color[2], blend_color[3]}

    FragUniform fragment_uniforms[] = {
        {
            .type = GkurveType_SemicircleConvex,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_SemicircleConvex,
            .blend_color = BLEND_COLOR,
        },
    };
    // append to app data
    vertexBufferAppendArray(&app->vertices, vertices, 18);
    fragUniformBufferAppendArray(&app->fragment_uniform_list, fragment_uniforms, 6);
}