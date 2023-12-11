#include "draw.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void drawEquilateralTriangle(App * app, vec2 position, float scale, FragUniform uniform){
    float triangle_height = scale * sqrtf(0.75);

    Vertex vertices[] = {
        { .pos = { position[0] + scale / 2, position[1] + triangle_height, 0, 1 }, .uv = { 0.5, 1 } },
        { .pos = { position[0], position[1], 0, 1 }, .uv = { 0, 0 } },
        { .pos = { position[0] + scale, position[1], 0, 1 }, .uv = { 1, 0 } },
    };
    
    // append to app data
    if (app->vertices == NULL){
        app->vertices = malloc(sizeof(vertices));
        memcpy(app->vertices, vertices, sizeof(vertices));
        app->vertices_size = sizeof(vertices);
    } else {
        app->vertices = realloc(app->vertices, app->vertices_size + sizeof(vertices));
        size_t offset = app->vertices_size / sizeof(Vertex);
        app->vertices_size += sizeof(vertices);
        memcpy(&app->vertices[offset], vertices, sizeof(vertices));
    };

    if (app->fragment_uniform_list == NULL){
        app->fragment_uniform_list = malloc(sizeof(uniform));
        memcpy(app->fragment_uniform_list, &uniform, sizeof(uniform));
        app->fragment_uniform_list_size = sizeof(uniform);
    } else {
        app->fragment_uniform_list = realloc(app->fragment_uniform_list, app->fragment_uniform_list_size + sizeof(uniform));
        size_t offset = app->fragment_uniform_list_size / sizeof(FragUniform);
        app->fragment_uniform_list_size += sizeof(uniform);
        memcpy(&app->fragment_uniform_list[offset], &uniform, sizeof(uniform));
    };
};

void drawQuad(App * app, vec2 position, vec2 scale, FragUniform uniform){
    Vertex vertices[] = {
        { .pos = { position[0], position[1] + scale[1], 0, 1 }, .uv = { 0, 1 } },
        { .pos = { position[0], position[1], 0, 1 }, .uv = { 0, 0 } },
        { .pos = { position[0] + scale[0], position[1], 0, 1 }, .uv = { 1, 0 } },

        { .pos = { position[0], position[1] + scale[1], 0, 1 }, .uv = { 0, 1 } },
        { .pos = { position[0] + scale[0], position[1] + scale[1], 0, 1 }, .uv = { 1, 1 } },
        { .pos = { position[0] + scale[0], position[1], 0, 1 }, .uv = { 1, 0 } },
    };
    FragUniform fragment_uniform_list[] = {uniform, uniform};


    // append to app data
    if (app->vertices == NULL){
        app->vertices = malloc(sizeof(vertices));
        memcpy(app->vertices, vertices, sizeof(vertices));
        app->vertices_size = sizeof(vertices);
    } else {
        app->vertices = realloc(app->vertices, app->vertices_size + sizeof(vertices));
        size_t offset = app->vertices_size / sizeof(Vertex);
        app->vertices_size += sizeof(vertices);
        memcpy(&app->vertices[offset], vertices, sizeof(vertices));
    };

    if (app->fragment_uniform_list == NULL){
        app->fragment_uniform_list = malloc(sizeof(fragment_uniform_list));
        memcpy(app->fragment_uniform_list, fragment_uniform_list, sizeof(fragment_uniform_list));
        app->fragment_uniform_list_size = sizeof(fragment_uniform_list);
    } else {
        app->fragment_uniform_list = realloc(app->fragment_uniform_list, app->fragment_uniform_list_size + sizeof(fragment_uniform_list));
        size_t offset = app->fragment_uniform_list_size / sizeof(FragUniform);
        app->fragment_uniform_list_size += sizeof(fragment_uniform_list);
        memcpy(&app->fragment_uniform_list[offset], fragment_uniform_list, sizeof(fragment_uniform_list));
    };
};

void drawCircle(App * app, vec2 position, float radius, vec4 blend_color){

    #define LOW_MID { position[0], position[1] - radius, 0, 1 }
    #define HIGH_MID { position[0], position[1] + radius, 0, 1 }

    #define MID_LEFT { position[0] - radius, position[1], 0, 1 }
    #define MID_RIGHT { position[0] + radius, position[1], 0, 1 }

    float p = 0.95 * radius;

    #define HIGH_RIGHT { position[0] + p, position[1] + p, 0, 1 }
    #define HIGH_LEFT { position[0] - p, position[1] + p, 0, 1 }
    #define LOW_RIGHT { position[0] + p, position[1] - p, 0, 1 }
    #define LOW_LEFT { position[0] - p, position[1] - p, 0, 1 }

    #define BLEND_COLOR {blend_color[0], blend_color[1], blend_color[2], blend_color[3]}

    Vertex vertices[] = {
        { .pos = LOW_MID, .uv = { 0.5, 0 } },
        { .pos = MID_RIGHT, .uv = { 0.5, 0 } },
        { .pos = HIGH_MID, .uv = { 0.5, 0 } },

        { .pos = HIGH_MID, .uv = { 0.5, 0 } },
        { .pos = MID_LEFT, .uv = { 0.5, 0 } },
        { .pos = LOW_MID, .uv = { 0.5, 0 } },

        { .pos = LOW_RIGHT, .uv = { 0.5, 0 } },
        { .pos = MID_RIGHT, .uv = { 0.5, 0 } },
        { .pos = LOW_MID, .uv = { 0.5, 0 } },

        { .pos = HIGH_RIGHT, .uv = { 0.5, 0 } },
        { .pos = HIGH_MID, .uv = { 0.5, 0 } },
        { .pos = MID_RIGHT, .uv = { 0.5, 0 } },

        { .pos = HIGH_LEFT, .uv = { 0.5, 0 } },
        { .pos = MID_LEFT, .uv = { 0.5, 0 } },
        { .pos = HIGH_MID, .uv = { 0.5, 0 } },

        { .pos = LOW_LEFT, .uv = { 0.5, 0 } },
        { .pos = LOW_MID, .uv = { 0.5, 0 } },
        { .pos = MID_LEFT, .uv = { 0.5, 0 } },
    };

    FragUniform fragment_uniform_list[] = {
        {
            .type = GkurveType_Filled,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_Filled,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_Convex,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_Convex,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_Convex,
            .blend_color = BLEND_COLOR,
        },
        {
            .type = GkurveType_Convex,
            .blend_color = BLEND_COLOR,
        },
    };
    // append to app data
    if (app->vertices == NULL){
        app->vertices = malloc(sizeof(vertices));
        memcpy(app->vertices, vertices, sizeof(vertices));
        app->vertices_size = sizeof(vertices);
    } else {
        app->vertices = realloc(app->vertices, app->vertices_size + sizeof(vertices));
        size_t offset = app->vertices_size / sizeof(Vertex);
        app->vertices_size += sizeof(vertices);
        memcpy(&app->vertices[offset], vertices, sizeof(vertices));
    };

    if (app->fragment_uniform_list == NULL){
        app->fragment_uniform_list = malloc(sizeof(fragment_uniform_list));
        memcpy(app->fragment_uniform_list, fragment_uniform_list, sizeof(fragment_uniform_list));
        app->fragment_uniform_list_size = sizeof(fragment_uniform_list);
    } else {
        app->fragment_uniform_list = realloc(app->fragment_uniform_list, app->fragment_uniform_list_size + sizeof(fragment_uniform_list));
        size_t offset = app->fragment_uniform_list_size / sizeof(FragUniform);
        app->fragment_uniform_list_size += sizeof(fragment_uniform_list);
        memcpy(&app->fragment_uniform_list[offset], fragment_uniform_list, sizeof(fragment_uniform_list));
    };



}