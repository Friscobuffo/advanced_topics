#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

int isInIntArray(int arr[], int num, int size) {
    for (int i = 0; i < size; i++)
        if (arr[i] == num)
            return 1;
    return 0;
}

void insertInIntArray(int* array, int size, int index, int value) {
    for (int i = size-1; i > index; i--) {
        array[i] = array[i-1];
    }
    array[index] = value;
}

// returns the position of the inserted value (-1 if not inserted)
// the sorting is descending
int insertInFloatSortedArray(float* array, int size, float value, int maxSize) {
    if (size == 0) {
        array[0] =  value;
        return 0;
    }
    int i = size-1;
    while (i >= 0 && array[i] < value) {
        if (i+1 != maxSize) 
            array[i+1] = array[i];
        i--;
    }
    if (i+1 == maxSize) return -1;
    array[i+1] = value;
    return i+1;
}

int posInIntOrderedArray(int* array, int size, int target) {
    for (int i = 0; i < size; i++) {
        if (array[i] == target)
            return i;
        if (array[i] > target)
            return -1;
    }
    return -1;
}

void printIntArray(int* array, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", array[i]);
        if (i < size-1)
            printf(", ");
    }
    printf("]\n");
}

void printFloatArray(float* array, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%.2f", array[i]);
        if (i < size-1) {
            printf(", ");
        }
    }
    printf("]\n");
}

float averageOfFloatArray(float* array, int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++)
        sum += array[i];
    float average = sum / size;
    return average;
}

int find_max_position_in_float_array(float* array, int size) {
    if (size <= 0)
        return -1;
    float max = array[0];
    int maxPosition = 0;
    for (int i = 1; i < size; i++) {
        if (array[i] > max) {
            max = array[i];
            maxPosition = i;
        }
    }
    return maxPosition;
}

// map

#define TABLE_SIZE 100

typedef struct {
    int key;
    float value;
} KeyValuePair;

typedef struct {
    KeyValuePair** table[TABLE_SIZE];
    int adjacencyListSize[TABLE_SIZE];
    int adjacencyListMaxSize[TABLE_SIZE];
} HashMap;

KeyValuePair* newKeyValuePair(int key, float value) {
    KeyValuePair* kvPair = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    kvPair->key = key;
    kvPair->value = value;
    return kvPair;
}

HashMap* createHashMap() {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->table[i] = NULL;
        map->adjacencyListSize[i] = 0;
    }
    return map;
}

int hash(int key) {
    return key % TABLE_SIZE;
}

void insertInHashMap(HashMap* map, int key, float value) {
    int index = hash(key);
    KeyValuePair* newPair = newKeyValuePair(key, value);
    if (map->table[index] == NULL) {
        map->table[index] = (KeyValuePair**)malloc(2*sizeof(KeyValuePair*));
        map->adjacencyListMaxSize[index] = 2;
    }
    int size = map->adjacencyListSize[index];
    int maxSize = map->adjacencyListMaxSize[index];
    if (size == maxSize) {
        map->table[index] = (KeyValuePair**)realloc(map->table[index], 2*maxSize*sizeof(KeyValuePair*));
        map->adjacencyListMaxSize[index] = 2*map->adjacencyListMaxSize[index];
    }
    map->table[index][size] = newPair;
    map->adjacencyListSize[index] = map->adjacencyListSize[index]+1;
}

float getValueFromHashMap(HashMap* map, int key) {
    int index = hash(key);
    int size = map->adjacencyListSize[index];
    KeyValuePair** adjList = map->table[index];
    for (int i = 0; i < size; i++) {
        KeyValuePair* current = adjList[i];
        if (current->key == key)
            return current->value;
    }
    return -1;
}

void freeHashMap(HashMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (map->table[i] != NULL) {
            int size = map->adjacencyListSize[i];
            free(map->table[i]);
        }
    }
    free(map);
}

void printHashMap(HashMap* map) {
    printf("{");
    for (int i = 0; i < TABLE_SIZE; i++) {
        KeyValuePair** adjList = map->table[i];
        if (adjList != NULL) {
            int size = map->adjacencyListSize[i];
            for (int j = 0; j < size; j++)
                printf("%d: %.2f, ", adjList[j]->key, adjList[j]->value);
        }
    }
    printf("}\n");
}

#endif