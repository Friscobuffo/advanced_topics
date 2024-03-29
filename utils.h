#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

// returns 1 if num is in array, 0 otherwise
int isInIntArray(int* array, int num, int size) {
    for (int i = 0; i < size; i++)
        if (array[i] == num)
            return 1;
    return 0;
}

// insert value in array in index position and shifts the other values to the right
void insertInIntArray(int* array, int size, int index, int value) {
    for (int i = size-1; i > index; i--) {
        array[i] = array[i-1];
    }
    array[index] = value;
}

/* inserts a value in a sorted array and shifts the other values
   returns the position of the inserted value (-1 if not inserted)
   the sorting is descending */
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

// returns position of target in sorted array, -1 if target is not present
// sorting is descending
int posInIntSortedArray(int* array, int size, int target) {
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

int findMaxPositionInFloatArray(float* array, int size) {
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

float findMinInFloatArray(float* array, int size) {
    if (size <= 0)
        return -1;
    float min = array[0];
    for (int i = 1; i < size; i++)
        if (array[i] < min)
            min = array[i];
    return min;
}

// return the index position of a value in an int array
// if element is not in array, returns -1
int posInIntArray(int* array, int size, int target) {
    for (int i = 0; i < size; i++)
        if (array[i] == target)
            return i;
    return -1;
}

// quick sort helper functions
void _swap(float* a, float* b) {
    float t = *a;
    *a = *b;
    *b = t;
}

int _partition(float* arr, int low, int high) {
    float pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            _swap(&arr[i], &arr[j]);
        }
    }
    _swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void _quick(float* arr, int low, int high) {
    if (low < high) {
        int pi = _partition(arr, low, high);
        _quick(arr, low, pi - 1);
        _quick(arr, pi + 1, high);
    }
}

// quick sort a float array
void quickSort(float arr[], int size) {
    _quick(arr, 0, size-1);
}

// map
#define TABLE_SIZE 100

typedef struct {
    int _key;
    float _value;
} KeyValuePair;

typedef struct {
    KeyValuePair** _table[TABLE_SIZE];
    int _adjacencyListSize[TABLE_SIZE];
    int _adjacencyListMaxSize[TABLE_SIZE];
} HashMap;

KeyValuePair* _newKeyValuePair(int key, float value) {
    KeyValuePair* kvPair = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    kvPair->_key = key;
    kvPair->_value = value;
    return kvPair;
}

HashMap* createHashMap() {
    HashMap* mapPtr = (HashMap*)malloc(sizeof(HashMap));
    for (int i = 0; i < TABLE_SIZE; i++) {
        mapPtr->_table[i] = NULL;
        mapPtr->_adjacencyListSize[i] = 0;
    }
    return mapPtr;
}

int _hash(int key) {
    return key % TABLE_SIZE;
}

void insertInHashMap(HashMap* mapPtr, int key, float value) {
    int index = _hash(key);
    KeyValuePair* newPair = _newKeyValuePair(key, value);
    if (mapPtr->_table[index] == NULL) {
        mapPtr->_table[index] = (KeyValuePair**)malloc(2*sizeof(KeyValuePair*));
        mapPtr->_adjacencyListMaxSize[index] = 2;
    }
    int size = mapPtr->_adjacencyListSize[index];
    int maxSize = mapPtr->_adjacencyListMaxSize[index];
    if (size == maxSize) {
        mapPtr->_table[index] = (KeyValuePair**)realloc(mapPtr->_table[index], 2*maxSize*sizeof(KeyValuePair*));
        mapPtr->_adjacencyListMaxSize[index] = 2*mapPtr->_adjacencyListMaxSize[index];
    }
    mapPtr->_table[index][size] = newPair;
    mapPtr->_adjacencyListSize[index] = mapPtr->_adjacencyListSize[index]+1;
}

float getValueFromHashMap(HashMap* mapPtr, int key) {
    int index = _hash(key);
    int size = mapPtr->_adjacencyListSize[index];
    KeyValuePair** adjList = mapPtr->_table[index];
    for (int i = 0; i < size; i++) {
        KeyValuePair* current = adjList[i];
        if (current->_key == key)
            return current->_value;
    }
    return -1;
}

void freeHashMap(HashMap* mapPtr) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (mapPtr->_table[i] != NULL) {
            int size = mapPtr->_adjacencyListSize[i];
            free(mapPtr->_table[i]);
        }
    }
    free(mapPtr);
}

void printHashMap(HashMap* mapPtr) {
    printf("{");
    for (int i = 0; i < TABLE_SIZE; i++) {
        KeyValuePair** adjList = mapPtr->_table[i];
        if (adjList != NULL) {
            int size = mapPtr->_adjacencyListSize[i];
            for (int j = 0; j < size; j++)
                printf("%d: %.2f, ", adjList[j]->_key, adjList[j]->_value);
        }
    }
    printf("}\n");
}

#endif