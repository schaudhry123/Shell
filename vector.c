// Shell
// CS 241 Fall 2015

#define _GNU_SOURCE 1

#include "vector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

Vector *Vector_create() {
    Vector *vtr = malloc(sizeof(Vector));
    vtr->size = 0;
    vtr->capacity = 10;
    vtr->array = malloc(sizeof(char *) * vtr->capacity);
    return vtr;
}

void Vector_destroy(Vector *vtr) {
    assertValidVector(vtr);
    for (int i = 0; i < (int)vtr->size; i++) {
        if (vtr->array[i] != NULL)
            free(vtr->array[i]);
    }
    vtr->size = 0;
    vtr->capacity = 0;
    free(vtr->array);
    free(vtr);
}

int Vector_size(Vector *vtr) {
    assertValidVector(vtr);
    return (int)vtr->size;
} 

void Vector_resize(Vector *vtr, size_t new_size) {
    assertValidVector(vtr);
    // If the array grows
    if (vtr->size < new_size) {
        if (vtr->capacity < new_size) {
            while (vtr->capacity < new_size) { // Ensure capacity > new_size            
                vtr->capacity *= 2;
            }
            vtr->array = realloc(vtr->array, sizeof(char *) * vtr->capacity);
        }
    	for (int i = vtr->size; i < (int)new_size; i++) {
    	    vtr->array[i] = NULL;
    	}
    }
    // If the array shrinks
    else {
    	for (int i = vtr->size-1; i >= (int)new_size; i--) {
            if (vtr->array[i] != NULL)
    	       free(vtr->array[i]);
    	}
    }
    vtr->size = new_size;
}

void Vector_set(Vector *vtr, size_t index, const char *str) {
    assertValidIndex(vtr, index);
    if (vtr->array[index] != NULL)
        free(vtr->array[index]);
    if (str == NULL)
	   vtr->array[index] = NULL;
    else
        vtr->array[index] = strdup(str);
}

const char *Vector_get(Vector *vtr, size_t index) {
    assertValidIndex(vtr, index);
    if (vtr->array[index] == NULL)//
	   return NULL;
    //fprintf(stderr, "Printing array[index] and &array[index] and *array[index]: %s %p %c\n", vtr->array[index], &(vtr->array[index]), *(vtr->array[index]));
    return vtr->array[index];
}

void Vector_append(Vector *vtr, const char *str) {
    assertValidVector(vtr);
    Vector_resize(vtr, vtr->size + 1);
    Vector_set(vtr, vtr->size-1, str);
}

void assertValidVector(Vector *vtr) {
    assert(vtr != NULL);
}

void assertValidIndex(Vector *vtr, size_t index) {
    assertValidVector(vtr);
    assert( index < vtr->size);
}

void printVector(Vector *vtr) {
    assertValidVector(vtr);
    fprintf(stderr, "Printing vector of size %zu and capacity %zu: \n", vtr->size, vtr->capacity);
    for (int i = 0; i < (int)vtr->size; i++) {
        fprintf(stderr, "\t%d:\t%s\n", i, vtr->array[i]);
    }
}

void removeNewLines(char * str) {
    //fprintf(stderr, "Printing: *str\n");
    if (str != NULL) {
        char *pos;
        if ((pos = strchr(str, '\n')) != NULL)
            *pos = '\0';
    }
}