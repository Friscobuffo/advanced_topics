#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct {
    int id;
    int* arrayMoviesIds;
    float* arrayMoviesRatings;
    int numRatings;
    float averageRating;
    HashMap* mapMovieIdRating;
} User;

typedef struct {
    int numUsers;
    User** arrayUsers;
} Dataset;

void populateUsersHashMaps(Dataset* datasetPtr) {
    for (int i = 0; i < datasetPtr->numUsers; i++) {
        User* user = datasetPtr->arrayUsers[i];
        HashMap* hashMap = createHashMap();
        user->mapMovieIdRating = hashMap;
        int numRatings = datasetPtr->arrayUsers[i]->numRatings;
        float* ratings = datasetPtr->arrayUsers[i]->arrayMoviesRatings;
        int* ids = datasetPtr->arrayUsers[i]->arrayMoviesIds;
        for (int j = 0; j < numRatings; j++)
            insertInHashMap(hashMap, ids[j], ratings[j]);
    }
}

void computeAveragesRatings(Dataset* datasetPtr) {
    for (int i = 0; i < datasetPtr->numUsers; i++) {
        float avg = 0.0;
        int numRatings = datasetPtr->arrayUsers[i]->numRatings;
        float* ratings = datasetPtr->arrayUsers[i]->arrayMoviesRatings;
        for (int j = 0; j < numRatings; j++)
            avg += ratings[j];
        avg = avg / numRatings;
        datasetPtr->arrayUsers[i]->averageRating = avg;
    }
}

Dataset* readCsv(char* file_path) {
    FILE *file;
    char line[1024];
    file = fopen(file_path, "r");
    int maxSize = 1024;
    int numUsers = 0;
    Dataset* dataset = (Dataset*)malloc(sizeof(Dataset));
    dataset->arrayUsers = (User**)malloc(maxSize*sizeof(User*));
    fgets(line, 1024, file); // skipping csv labels row
    // getting first user
    fgets(line, 1024, file);
    line[strcspn(line, "\n")] = '\0';
    char* token = strtok(line, ",");
    int prevUserId = atoi(token);
    token = strtok(NULL, ",");
    int movieId = atoi(token);
    token = strtok(NULL, ",");
    float rating = atof(token);

    int* arrayPrevUserMoviesIds = (int*)malloc(256*sizeof(int));
    float* arrayPrevUserRatings = (float*)malloc(256*sizeof(float));
    int bufferMaxSize = 256;
    int prevUserNumRatings = 1;
    arrayPrevUserMoviesIds[0] = movieId;
    arrayPrevUserRatings[0] = rating;

    while (fgets(line, 1024, file)) {
        line[strcspn(line, "\n")] = '\0';
        char* token = strtok(line, ",");
        int userId = atoi(token);
        token = strtok(NULL, ",");
        int movieId = atoi(token);
        token = strtok(NULL, ",");
        float rating = atof(token);

        if (userId == prevUserId) { // adding movie to prev user
            if (prevUserNumRatings == bufferMaxSize) {
                arrayPrevUserMoviesIds = (int*)realloc(arrayPrevUserMoviesIds, 2*bufferMaxSize*sizeof(int));
                arrayPrevUserRatings = (float*)realloc(arrayPrevUserRatings, 2*bufferMaxSize*sizeof(float));
                bufferMaxSize *= 2;
            }
            arrayPrevUserMoviesIds[prevUserNumRatings] = movieId;
            arrayPrevUserRatings[prevUserNumRatings] = rating;
            prevUserNumRatings++;
        }
        else { // adding prev user
            if (numUsers == maxSize) {
                dataset->arrayUsers = (User**)realloc(dataset->arrayUsers, 2*maxSize*sizeof(User*));
                maxSize = maxSize*2;
            }
            User* user = (User*)malloc(sizeof(User));
            user->id = prevUserId;
            user->numRatings = prevUserNumRatings;
            user->arrayMoviesIds = (int*)malloc(prevUserNumRatings*sizeof(int));
            user->arrayMoviesRatings = (float*)malloc(prevUserNumRatings*sizeof(float));
            for (int i = 0; i < prevUserNumRatings; i++) {
                user->arrayMoviesIds[i] = arrayPrevUserMoviesIds[i];
                user->arrayMoviesRatings[i] = arrayPrevUserRatings[i];
            }
            dataset->arrayUsers[numUsers] = user;
            numUsers++;
            prevUserId = userId;
            prevUserNumRatings = 1;
            arrayPrevUserMoviesIds[0] = movieId;
            arrayPrevUserRatings[0] = rating;
        }
    }
    // adding last user
    if (numUsers == maxSize) {
        dataset->arrayUsers = (User**)realloc(dataset->arrayUsers, 1+maxSize*sizeof(User*));
        maxSize++;
    }
    User* user = (User*)malloc(sizeof(User));
    user->id = prevUserId;
    user->numRatings = prevUserNumRatings;
    user->arrayMoviesIds = (int*)malloc(prevUserNumRatings*sizeof(int));
    user->arrayMoviesRatings = (float*)malloc(prevUserNumRatings*sizeof(float));
    for (int i = 0; i < prevUserNumRatings; i++) {
        user->arrayMoviesIds[i] = arrayPrevUserMoviesIds[i];
        user->arrayMoviesRatings[i] = arrayPrevUserRatings[i];
    }

    dataset->arrayUsers[numUsers] = user;
    numUsers++;
    dataset->numUsers = numUsers;
    dataset->arrayUsers = (User**)realloc(dataset->arrayUsers, numUsers*sizeof(User*));
    
    free(arrayPrevUserMoviesIds);
    free(arrayPrevUserRatings);
    fclose(file);
    computeAveragesRatings(dataset);
    populateUsersHashMaps(dataset);
    return dataset;
}

User* getUserById(Dataset* dataset, int id) {
    return dataset->arrayUsers[id-1];
}

void freeUser(User* user) {
    free(user->arrayMoviesIds);
    free(user->arrayMoviesRatings);
    freeHashMap(user->mapMovieIdRating);
    free(user);
}

void freeDataset(Dataset* datasetPtr) {
    for (int i = 0; i < datasetPtr->numUsers; i++)
        freeUser(datasetPtr->arrayUsers[i]);
    free(datasetPtr->arrayUsers);
    free(datasetPtr);
}

#endif