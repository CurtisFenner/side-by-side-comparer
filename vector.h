// Curtis Fenner, 2017

#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define SRC __FILE__ ":" STRINGIFY(__LINE__)

static void assertMessage_(bool test, char const* condition, char const* source) {
	if (!test) {
		printf("assert failed!!! %s:\n\t%s\n", source, condition);
		exit(EXIT_FAILURE);
	}
}

#define assertMessage(test, source) assertMessage_(test, #test, source)

#define type_pair(aType, bType, name) typedef struct { aType left; bType right; } name; \
name* name##_make(aType a, bType b) { \
	name* out = (name*)malloc(sizeof(name)); \
	out->left = a; \
	out->right = b; \
	return out; \
}

#define type_vector(elementType, vname) typedef struct { \
	size_t count; size_t capacity; elementType* elements; \
} vname; \
vname* vname##_make() /* int_vector_make() */ {  \
	vname* out = (vname*)malloc(sizeof(vname)); \
	out->count = 0; \
	out->capacity = 0; \
	out->elements = NULL; \
	return out; \
} \
void vname##_append(vname* vector, elementType element) /* int_vector_append(int_vector*, int) */ { \
	if (vector->count == vector->capacity) { \
		elementType* old = vector->elements; \
		vector->capacity = vector->capacity * 2 + 1; \
		vector->elements = (elementType*)malloc(vector->capacity * sizeof(elementType)); \
		if (vector->capacity > 1) {\
			memcpy(vector->elements, old, sizeof(elementType) * vector->count); \
			free(old); \
		} \
	} \
	vector->elements[vector->count++] = element; \
} \
elementType vname##_get(vname const* vector, size_t index, const char* source) /* int_vector_get(int_vector*, int) */ { \
	assert(index + 1 != 0); \
	assertMessage(index < vector->count, source); \
	return vector->elements[index]; \
} \
size_t vname##_size(vname const* vector) /* size_t int_vector_size(int_vector const*) */ { return vector->count; } \
elementType vname##_pop(vname* vector) /* int int_vector_pop(int_vector*) */ { \
	assert(vector->count > 0); \
	return vector->elements[--vector->count]; \
}\
elementType vname##_last(vname const* vector) /* int int_vector_last(int_vector const*) */ { \
	assert(vector->count > 0); \
	return vector->elements[vector->count-1]; \
} \
void vname##_release(vname* vector) /* void int_vector_release(int_vector*) */ {\
	if (vector->capacity > 0) { \
		free(vector->elements); \
	} \
	free(vector); \
}\

// EXAMPLE:
//type_vector(vector_int, int);
// vector_int* myInts = vector_int_make();
// vector_int_append(myInts, 7);
// vector_int_append(myInts, 12);
// vector_int_append(myInts, 5);
// int foo = vector_int_get(myInts, 1); // --> 12
// size_t count = vector_int_size(myInts); // --> 1
// int last = vector_pop(int, myInts); // --> 5
// vector_release(int, myInts);
