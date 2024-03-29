#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// user struct
typedef struct {
    int id;
    int* arrayMoviesIds;
    float* arrayMoviesRatings;
    int numRatings;
    float averageRating;
    HashMap* mapMovieIdRating;
} User;

// dataset struct
typedef struct {
    int numUsers;
    User** arrayUsers;
} Dataset;

// for each user in the dataset, it populates his hash map with
// movies ids as keys and ratings as values
void _populateUsersHashMaps(Dataset* datasetPtr) {
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

// computes average ratings for each user in the dataset
void _computeAveragesRatings(Dataset* datasetPtr) {
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

/* populates the dataset struct with data from csv
   assumes each csv line is at most 1024 chars long
   assumes csv is in format <userId>,<movieId>,<rating>,<timestamp>
   (but timestamps are actually skipped)
*/
Dataset* readCsv(char* file_path) {
    FILE *file;
    char line[1024];
    file = fopen(file_path, "r");
    int maxSize = 1024;
    int numUsers = 0;
    Dataset* datasetPtr = (Dataset*)malloc(sizeof(Dataset));
    datasetPtr->arrayUsers = (User**)malloc(maxSize*sizeof(User*));
    fgets(line, 1024, file); // skipping csv labels row (the first row)
    // getting first user
    fgets(line, 1024, file);
    line[strcspn(line, "\n")] = '\0';
    char* token = strtok(line, ",");
    int prevUserId = atoi(token);
    token = strtok(NULL, ",");
    int movieId = atoi(token);
    token = strtok(NULL, ",");
    float rating = atof(token);

    int bufferMaxSize = 256;
    int* arrayPrevUserMoviesIds = (int*)malloc(bufferMaxSize*sizeof(int));
    float* arrayPrevUserRatings = (float*)malloc(bufferMaxSize*sizeof(float));
    int prevUserNumRatings = 1;
    arrayPrevUserMoviesIds[0] = movieId;
    arrayPrevUserRatings[0] = rating;

    while (fgets(line, 1024, file)) {
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, ",");
        int userId = atoi(token);
        token = strtok(NULL, ",");
        int movieId = atoi(token);
        token = strtok(NULL, ",");
        float rating = atof(token);

        if (userId == prevUserId) { // if its still the prev user we add the movie infos
            if (prevUserNumRatings == bufferMaxSize) {
                bufferMaxSize *= 2;
                arrayPrevUserMoviesIds = (int*)realloc(arrayPrevUserMoviesIds, bufferMaxSize*sizeof(int));
                arrayPrevUserRatings = (float*)realloc(arrayPrevUserRatings, bufferMaxSize*sizeof(float));
            }
            arrayPrevUserMoviesIds[prevUserNumRatings] = movieId;
            arrayPrevUserRatings[prevUserNumRatings] = rating;
            prevUserNumRatings++;
        }
        else { // if the user is new we add prev user and new user becomes prev user
            if (numUsers == maxSize) {
                maxSize *= 2;
                datasetPtr->arrayUsers = (User**)realloc(datasetPtr->arrayUsers, maxSize*sizeof(User*));
            }
            User* userPtr = (User*)malloc(sizeof(User));
            userPtr->id = prevUserId;
            userPtr->numRatings = prevUserNumRatings;
            userPtr->arrayMoviesIds = (int*)malloc(prevUserNumRatings*sizeof(int));
            userPtr->arrayMoviesRatings = (float*)malloc(prevUserNumRatings*sizeof(float));
            for (int i = 0; i < prevUserNumRatings; i++) {
                userPtr->arrayMoviesIds[i] = arrayPrevUserMoviesIds[i];
                userPtr->arrayMoviesRatings[i] = arrayPrevUserRatings[i];
            }
            datasetPtr->arrayUsers[numUsers] = userPtr;
            numUsers++;
            prevUserId = userId;
            prevUserNumRatings = 1;
            arrayPrevUserMoviesIds[0] = movieId;
            arrayPrevUserRatings[0] = rating;
        }
    }
    // adding last user
    if (numUsers == maxSize) {
        maxSize++;
        datasetPtr->arrayUsers = (User**)realloc(datasetPtr->arrayUsers, maxSize*sizeof(User*));
    }
    User* userPtr = (User*)malloc(sizeof(User));
    userPtr->id = prevUserId;
    userPtr->numRatings = prevUserNumRatings;
    userPtr->arrayMoviesIds = (int*)malloc(prevUserNumRatings*sizeof(int));
    userPtr->arrayMoviesRatings = (float*)malloc(prevUserNumRatings*sizeof(float));
    for (int i = 0; i < prevUserNumRatings; i++) {
        userPtr->arrayMoviesIds[i] = arrayPrevUserMoviesIds[i];
        userPtr->arrayMoviesRatings[i] = arrayPrevUserRatings[i];
    }
    datasetPtr->arrayUsers[numUsers] = userPtr;
    numUsers++;
    datasetPtr->numUsers = numUsers;
    datasetPtr->arrayUsers = (User**)realloc(datasetPtr->arrayUsers, numUsers*sizeof(User*));
    
    free(arrayPrevUserMoviesIds);
    free(arrayPrevUserRatings);
    fclose(file);
    _computeAveragesRatings(datasetPtr);
    _populateUsersHashMaps(datasetPtr);
    return datasetPtr;
}

User* getUserById(Dataset* datasetPtr, int id) {
    return datasetPtr->arrayUsers[id-1];
}

void _freeUser(User* userPtr) {
    free(userPtr->arrayMoviesIds);
    free(userPtr->arrayMoviesRatings);
    freeHashMap(userPtr->mapMovieIdRating);
    free(userPtr);
}

void freeDataset(Dataset* datasetPtr) {
    for (int i = 0; i < datasetPtr->numUsers; i++)
        _freeUser(datasetPtr->arrayUsers[i]);
    free(datasetPtr->arrayUsers);
    free(datasetPtr);
}

#endif