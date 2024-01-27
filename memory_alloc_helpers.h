#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "draw_structs.h"

#ifndef MEMORY_ALLOC_HELPERS_
#define MEMORY_ALLOC_HELPERS_


// This is the max size allocate for these buffers. 
// Most Operating Systems malloc won't use the whole size
static const size_t MAX_MEMORY_BUFFER_SIZE = (1024*1024*1024); // 1GB;

typedef struct IntBuffer {
    int * items;
    unsigned int length;
    size_t max_length;
} IntBuffer;

typedef struct U16Buffer {
    u_int16_t * items;
    unsigned int length;
    size_t max_length;
} U16Buffer;

typedef struct Vec2Buffer {
    vec2 * items;
    unsigned int length;
    size_t max_length;
} Vec2Buffer;

typedef struct Vec2BufferBuffer {
    Vec2Buffer * items;
    unsigned int length;
    size_t max_length;
} Vec2BufferBuffer;

typedef struct VertexBuffer {
    Vertex * items;
    unsigned int length;
    size_t max_length;
} VertexBuffer;

typedef struct FragUniformBuffer {
    FragUniform * items;
    unsigned int length;
    size_t max_length;
} FragUniformBuffer;

/* initialize buffer */
IntBuffer intBufferInit();
/* free the malloced data and set everything to 0 */
void intBufferDestroy(IntBuffer * buffer);
/* append item to buffer */
void intBufferAppend(IntBuffer * buffer, int * item);

/* initialize buffer */
U16Buffer u16BufferInit();
/* free the malloced data and set everything to 0 */
void u16BufferDestroy(U16Buffer * buffer);
/* append item to buffer */
void u16BufferAppend(U16Buffer * buffer, u_int16_t * item);
/* append array to buffer*/
void u16BufferAppendArray(U16Buffer * buffer, u_int16_t * array, size_t length);

/* initialize buffer */
Vec2Buffer vec2BufferInit();
/* clear and reset buffer to initilized value*/
void vec2BufferClear(Vec2Buffer * buffer);
/* free the malloced data and set everything to 0 */
void vec2BufferDestroy(Vec2Buffer * buffer);
/* append item to buffer */
void vec2BufferAppend(Vec2Buffer * buffer, vec2 * item);
/* append array to buffer*/
void vec2BufferAppendArray(Vec2Buffer * buffer, vec2 * array, size_t length);
/* insert array into buffer at index*/
void vec2BufferInsertArray(Vec2Buffer * buffer, unsigned int index, vec2 * array, size_t length);

/* initialize buffer */
Vec2BufferBuffer vec2BufferBufferInit();
/* call detroy on all the buffers and free the malloced data and set everything to 0 */
void vec2BufferBufferDestroy(Vec2BufferBuffer * buffer);
/* append item to buffer */
void vec2BufferBufferAppend(Vec2BufferBuffer * buffer, Vec2Buffer * item);

/* initialize buffer */
VertexBuffer vertexBufferInit();
/* free the malloced data and set everything to 0 */
void vertexBufferDestory(VertexBuffer * buffer);
/* append item to buffer */
void vertexBufferAppend(VertexBuffer * buffer, Vertex * item);
/* append array to buffer*/
void vertexBufferAppendArray(VertexBuffer * buffer, Vertex * array, size_t length);

/* initialize buffer */
FragUniformBuffer fragUniformBufferInit();
/* append item to buffer */
void fragUniformBufferAppend(FragUniformBuffer * buffer, FragUniform * item);
/* append array to buffer*/
void fragUniformBufferAppendArray(FragUniformBuffer * buffer, FragUniform * array, size_t length);
#endif