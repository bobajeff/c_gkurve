#include "cglm/types.h"
#include "tesselator.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cglm/cglm.h>
#include "triangulate_polygon.h"

void* memAlloc(void* userData, unsigned int size)
{
	TESS_NOTUSED(userData);
	return malloc(size);
}

void memFree(void* userData, void* ptr)
{
	TESS_NOTUSED(userData);
	free(ptr);
}

#define NUM_COORDINATE_PER_VERTEX 2
int triangulatePolygon(Vec2Buffer polygon, IntBuffer * triangles){

	TESStesselator* tesselator = tessNewTess(&(TESSalloc){
		.memalloc = memAlloc,
		.memfree = memFree,
		.extraVertices = 255
	});
	assert(tesselator);

	tessAddContour(tesselator, NUM_COORDINATE_PER_VERTEX, polygon.items, sizeof(vec2), polygon.length);

	int vertices_per_element = 3; // Output Triangles
	int tesselate_return = tessTesselate(tesselator, TESS_WINDING_POSITIVE, TESS_POLYGONS, vertices_per_element, NUM_COORDINATE_PER_VERTEX, 0);
	if (tesselate_return == 0){
		fprintf(stderr, "Tesselate failed\n");
		assert(0);
	}
	
    const int *vertex_indices = tessGetVertexIndices(tesselator);
    const int *elements = tessGetElements(tesselator);
    const int element_count = tessGetElementCount(tesselator);

	int all_vertices_count = element_count * vertices_per_element;
	int i;
	for (i = 0; i < all_vertices_count; i++) {
		if (elements[i] != TESS_UNDEF){
			int index_value = vertex_indices[elements[i]];
			intBufferAppend(triangles, &index_value);
		}
	}

	// cleanup
	tessDeleteTess(tesselator);
    return 0;
}
