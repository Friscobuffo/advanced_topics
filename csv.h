#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Csv {
    int size;
    char** rows;
    int n_fields;
};

typedef struct Csv Csv;

Csv* read_csv(char* file_path) {
    FILE *file;
    char line[1024];
    file = fopen(file_path, "r");
    int max_size = 1024;
    int size = 0;
    Csv* csv = (Csv*)malloc(sizeof(Csv));
    csv->rows = (char**)malloc(max_size*sizeof(char*));
    while (fgets(line, 1024, file)) {
        line[strcspn(line, "\n")] = '\0';
        char* row = (char*)malloc(sizeof(char)*(strlen(line)+1));
        strcpy(row, line);
        if (size == max_size) {
            csv->rows = (char**)realloc(csv->rows, 2*max_size*sizeof(char*));
            max_size = max_size*2;
        }
        csv->rows[size] = row;
        size++;
    }
    csv->size = size;
    int n_fields = 1;
    for (int i=0; line[i]!='\0'; i++)
        if (line[i] == ',') n_fields++;
    csv->n_fields = n_fields;
    fclose(file);
    return csv;
}

char* get_row_field(Csv* csv, int field_index, int row_index) {
    if (field_index>=csv->n_fields) {
        printf("field index out of bounds\n");
        return NULL;
    }
    if (row_index>=csv->size) {
        printf("row index out of bounds\n");
        return NULL;
    }
    char* row_copy = strdup(csv->rows[row_index]);
    if (row_copy == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    char* token = strtok(row_copy, ",");
    int field_count = 0;
    while (token != NULL && field_count < field_index) {
        token = strtok(NULL, ",");
        field_count++;
    }
    char* output = (char*)malloc(sizeof(char)*(strlen(token)+1));
    strcpy(output, token);
    free(row_copy);
    return output;
}

void free_csv(Csv* csv_ptr) {
    for (int i = 0; i < csv_ptr->size; i++)
        free(csv_ptr->rows[i]);
    free(csv_ptr->rows);
    free(csv_ptr);
}

#endif