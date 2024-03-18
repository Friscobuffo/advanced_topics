#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

int is_in_int_array(int arr[], int num, int size) {
    for (int i = 0; i < size; i++)
        if (arr[i] == num)
            return 1;
    return 0;
}

void insert_in_int_array(int* array, int size, int index, int value) {
    for (int i = size-1; i > index; i--) {
        array[i] = array[i-1];
    }
    array[index] = value;
}

void insert_in_float_array(float* array, int size, int index, float value) {
    for (int i = size-1; i > index; i--) {
        array[i] = array[i-1];
    }
    array[index] = value;
}

float average_of_float_array(float* array, int size) {
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

float find_min_in_float_array(float* array, int size) {
    if (size <= 0)
        return -1;
    float min = array[0];
    for (int i = 1; i < size; i++)
        if (array[i] < min)
            min = array[i];
    return min;
}

// returns the position of the inserted value (-1 if not inserted)
// the sorting is descending
int insert_in_float_sorted_array(float* array, int last_elem_index, float value, int max_size) {
    if (last_elem_index == -1) {
        array[0] =  value;
        return 0;
    }
    int i = last_elem_index;
    while (i >= 0 && array[i] < value) {
        if (i+1 != max_size) 
            array[i+1] = array[i];
        i--;
    }
    if (i+1 == max_size) return -1;
    array[i+1] = value;
    return i+1;
}

// return the index of a value in an int array
int pos_in_int_array(int* array, int size, int target) {
    for (int i = 0; i < size; i++)
        if (array[i] == target)
            return i;
    return -1;
}

void print_int_array(int* array, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", array[i]);
        if (i < size-1)
            printf(", ");
    }
    printf("]\n");
}

void print_float_array(float* array, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%.2f", array[i]);
        if (i < size-1) {
            printf(", ");
        }
    }
    printf("]\n");
}

#endif