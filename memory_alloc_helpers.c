#include "memory_alloc_helpers.h"
#include <string.h>


IntBuffer intBufferInit(){
    size_t stride = sizeof(int);
    IntBuffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void intBufferDestroy(IntBuffer * buffer){
    if (buffer->items != NULL)
    {
        free(buffer->items);
    }
    buffer->items = NULL;
    buffer->length = 0;
    buffer->max_length = 0;
}

void intBufferAppend(IntBuffer * buffer, int * item){
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    buffer->items[buffer->length] = *item;
    buffer->length = new_length;
}

U16Buffer u16BufferInit(){
    size_t stride = sizeof(u_int16_t);
    U16Buffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void u16BufferDestroy(U16Buffer * buffer){
    if (buffer->items != NULL)
    {
        free(buffer->items);
    }
    buffer->items = NULL;
    buffer->length = 0;
    buffer->max_length = 0;
}

void u16BufferAppend(U16Buffer * buffer, u_int16_t * item){
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    buffer->items[buffer->length] = *item;
    buffer->length = new_length;
}

void u16BufferAppendArray(U16Buffer * buffer, u_int16_t * array, size_t length){
    size_t stride = sizeof(u_int16_t);
    size_t new_length = buffer->length + length;
    assert(new_length <= buffer->max_length);

    memcpy(&buffer->items[buffer->length], array, length * stride);
    buffer->length = new_length;
}

Vec2Buffer vec2BufferInit(){
    size_t stride = sizeof(vec2);
    Vec2Buffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void vec2BufferClear(Vec2Buffer * buffer){
    size_t stride = sizeof(vec2);
    memset(buffer->items, 0, stride * buffer->length);
    buffer->length = 0;
}

void vec2BufferDestroy(Vec2Buffer * buffer){
    if (buffer->items != NULL)
    {
        free(buffer->items);
    }
    buffer->items = NULL;
    buffer->length = 0;
    buffer->max_length = 0;
}

void vec2BufferAppend(Vec2Buffer * buffer, vec2 * item){
    size_t stride = sizeof(vec2);
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    memcpy(&buffer->items[buffer->length], item, stride);
    buffer->length = new_length;
}

void vec2BufferAppendArray(Vec2Buffer * buffer, vec2 * array, size_t length){
    size_t stride = sizeof(vec2);
    size_t new_length = buffer->length + length;
    assert(new_length <= buffer->max_length);

    memcpy(&buffer->items[buffer->length], array, length * stride);
    buffer->length = new_length;
}

void vec2BufferInsertArray(Vec2Buffer * buffer, unsigned int index, vec2 * array, size_t length){
    size_t stride = sizeof(vec2);
    size_t new_length = buffer->length + length;
    assert(new_length <= buffer->max_length);

    //move existing items at index to after the inserted items 
    size_t num_items_at_index = buffer->length - index;
    if (num_items_at_index > 0){
        size_t temp_size = num_items_at_index * stride;
        vec2 * temp = malloc(temp_size);
        memcpy(temp, &buffer->items[index], temp_size);
        memcpy(&buffer->items[new_length - num_items_at_index], temp, temp_size);
        free(temp);
    }
    memcpy(&buffer->items[index], array, length * stride);
    buffer->length = new_length;
}

Vec2BufferBuffer vec2BufferBufferInit(){
    size_t stride = sizeof(Vec2Buffer);
    Vec2BufferBuffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void vec2BufferBufferDestroy(Vec2BufferBuffer * buffer){
    int i;
    for (i = 0; i < buffer->length; i++){
        vec2BufferDestroy(&buffer->items[i]);
    }
    if (buffer->items != NULL)
    {
        free(buffer->items);
    }
    buffer->items = NULL;
    buffer->length = 0;
    buffer->max_length = 0;
}

void vec2BufferBufferAppend(Vec2BufferBuffer * buffer, Vec2Buffer * item){
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    buffer->items[buffer->length] = *item;
    buffer->length = new_length;
}

VertexBuffer vertexBufferInit(){
    size_t stride = sizeof(Vertex);
    VertexBuffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void vertexBufferDestory(VertexBuffer * buffer){
    if (buffer->items != NULL)
    {
        free(buffer->items);
    }
    buffer->items = NULL;
    buffer->length = 0;
    buffer->max_length = 0;
}

void vertexBufferAppend(VertexBuffer * buffer, Vertex * item){
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    buffer->items[buffer->length] = *item;
    buffer->length = new_length;
}

void vertexBufferAppendArray(VertexBuffer * buffer, Vertex * array, size_t length){
    size_t stride = sizeof(Vertex);
    size_t new_length = buffer->length + length;
    assert(new_length <= buffer->max_length);

    memcpy(&buffer->items[buffer->length], array, length * stride);
    buffer->length = new_length;
}

FragUniformBuffer fragUniformBufferInit(){
    size_t stride = sizeof(FragUniform);
    FragUniformBuffer mem = {.length = 0};
    mem.max_length = MAX_MEMORY_BUFFER_SIZE / stride;
    mem.items = malloc(mem.max_length * stride);
    assert(mem.items != NULL);
    return mem;
}

void fragUniformBufferAppend(FragUniformBuffer * buffer, FragUniform * item){
    size_t new_length = buffer->length + 1;
    assert(new_length <= buffer->max_length);

    buffer->items[buffer->length] = *item;
    buffer->length = new_length;
};

void fragUniformBufferAppendArray(FragUniformBuffer * buffer, FragUniform * array, size_t length){
    size_t stride = sizeof(FragUniform);
    size_t new_length = buffer->length + length;
    assert(new_length <= buffer->max_length);

    memcpy(&buffer->items[buffer->length], array, length * stride);
    buffer->length = new_length;
};