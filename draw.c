#include "draw.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void drawEquilateralTriangle(App * app, vec2 position, float scale, FragUniform uniform, UVData uv_data){
    float triangle_height = scale * sqrtf(0.75);

    Vertex vertices[] = {
        { .pos = { position[0] + scale / 2, position[1] + triangle_height, 0, 1 }},
        { .pos = { position[0], position[1], 0, 1 }},
        { .pos = { position[0] + scale, position[1], 0, 1 }},
    };
    // calculate and add uv_data
    vec2 temp;
    glm_vec2_mul(uv_data.width_and_height, (vec2){0.5, 1}, temp);
    glm_vec2_add(temp, uv_data.bottom_left, vertices[0].uv);
    glm_vec2_copy(uv_data.bottom_left, vertices[1].uv);
    glm_vec2_mul(uv_data.width_and_height, (vec2){1, 0 }, temp);
    glm_vec2_add(temp, uv_data.bottom_left, vertices[2].uv);

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

void drawQuad(App * app, vec2 position, vec2 scale, FragUniform uniform, UVData uv_data){
    Vertex vertices[] = {
        { .pos = { position[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0], position[1], 0, 1 }},
        { .pos = { position[0] + scale[0], position[1], 0, 1 }},

        { .pos = { position[0] + scale[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0], position[1] + scale[1], 0, 1 }},
        { .pos = { position[0] + scale[0], position[1], 0, 1 }},
    };

    vec2 temp;

    vec2 bottom_right_uv;
    vec2 up_left_uv;
    vec2 up_right_uv;

    glm_vec2_add(uv_data.bottom_left, uv_data.width_and_height, up_right_uv);
    glm_vec2_mul(uv_data.width_and_height, (vec2){1, 0}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, bottom_right_uv);
    glm_vec2_mul(uv_data.width_and_height, (vec2){0, 1}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, up_left_uv);

    glm_vec2_copy(up_left_uv, vertices[0].uv);
    glm_vec2_copy(uv_data.bottom_left, vertices[1].uv);
    glm_vec2_copy(bottom_right_uv, vertices[2].uv);

    glm_vec2_copy(up_right_uv, vertices[3].uv);
    glm_vec2_copy(up_left_uv, vertices[4].uv);
    glm_vec2_copy(bottom_right_uv, vertices[5].uv);

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

void drawCircle(App * app, vec2 position, float radius, vec4 blend_color, UVData uv_data){

    vec2 temp;

    Vertex low_mid = {.pos = {position[0], position[1] - radius, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){0.5, 0}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, low_mid.uv);
    Vertex high_mid = {.pos = {position[0], position[1] + radius, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){0.5, 1}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, high_mid.uv);
    Vertex mid_left = {.pos = {position[0] - radius, position[1], 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){0, 0.5}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, mid_left.uv);
    Vertex mid_right = {.pos = {position[0] + radius, position[1], 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){1, 0.5}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, mid_right.uv);

    float p = 0.95 * radius;

    Vertex high_right = {.pos = {position[0] + p, position[1] + p, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){1, 0.75}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, high_right.uv);
    Vertex high_left = {.pos = {position[0] - p, position[1] + p, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){0, 0.75}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, high_left.uv);
    Vertex low_right = {.pos = {position[0] + p, position[1] - p, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){1, 0.25}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, low_right.uv);
    Vertex low_left = {.pos = {position[0] - p, position[1] - p, 0, 1}};
    glm_vec2_mul(uv_data.width_and_height, (vec2){0, 0.25}, temp);
    glm_vec2_add(uv_data.bottom_left, temp, low_left.uv);

    #define BLEND_COLOR {blend_color[0], blend_color[1], blend_color[2], blend_color[3]}

    Vertex vertices[] = {
        low_mid,
        mid_right,
        high_mid,

        high_mid,
        mid_left,
        low_mid,

        low_right,
        mid_right,
        low_mid,

        high_right,
        high_mid,
        mid_right,

        high_left,
        mid_left,
        high_mid,

        low_left,
        low_mid,
        mid_left
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