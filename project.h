#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

float computePearsonCorrelation(User* user1ptr, User* user2ptr) {
    if (user1ptr->numRatings > user2ptr->numRatings) {
        User* temp = user1ptr;
        user1ptr = user2ptr;
        user2ptr = temp;
    }
    int user1numRatings = user1ptr->numRatings;
    int* arrayUser1moviesIds = user1ptr->arrayMoviesIds;
    float* arrayUser1ratings = user1ptr->arrayMoviesRatings;

    int sumUser1ratings = 0; // of movies in common
    int sumUser2ratings = 0; // of movies in common
    float numMoviesInCommon = 0;
    for (int j = 0; j < user1numRatings; j++) {
        int movieId = arrayUser1moviesIds[j];
        int ratingUser2 = getValueFromHashMap(user2ptr->mapMovieIdRating, movieId);
        if (ratingUser2 == -1) continue;
        sumUser1ratings += arrayUser1ratings[j];
        sumUser2ratings += ratingUser2;
        numMoviesInCommon++;
    }
    float avgUser1ratings = sumUser1ratings / numMoviesInCommon; // of movies in common
    float avgUser2ratings = sumUser2ratings / numMoviesInCommon; // of movies in common

    float numerator = 0;
    float denominator1 = 0;
    float denominator2 = 0;
    for (int j = 0; j < user1numRatings; j++) {
        int movieId = arrayUser1moviesIds[j];
        int y = getValueFromHashMap(user2ptr->mapMovieIdRating, movieId);
        if (y == -1) continue;
        float x = arrayUser1ratings[j];
        numerator += (x-avgUser1ratings)*(y-avgUser2ratings);
        denominator1 += (x-avgUser1ratings)*(x-avgUser1ratings);
        denominator2 += (y-avgUser2ratings)*(y-avgUser2ratings);
    }
    return numerator / sqrt(denominator1*denominator2);
}

float computeSimilarityBetweenUsersById(Dataset* datasetPtr, int user1id, int user2id, 
                                        float (*functionComputeSimilarity)(User*, User*)) {
    return functionComputeSimilarity(getUserById(datasetPtr, user1id), getUserById(datasetPtr, user2id));
}

void computeMostSimilarUsers(int numUsers, Dataset* datasetPtr, int userId, int* arrayTopUsersIds, float* arrayTopUserScores,
                            float (*functionComputeSimilarity)(User*, User*)) {
    int numAddedMovies = 0;
    User* user = getUserById(datasetPtr, userId);
    for (int i = 0; i < datasetPtr->numUsers; i++) {
        User* newUser = datasetPtr->arrayUsers[i];
        if (newUser->id == userId) continue;
        float score = functionComputeSimilarity(user, newUser);
        if (!isnan(score)) {
            int pos = insertInFloatSortedArray(arrayTopUserScores, numAddedMovies, score, numUsers);
            if (pos != -1) {
                insertInIntArray(arrayTopUsersIds, numUsers, pos, newUser->id);
                if (numAddedMovies != numUsers) numAddedMovies++;
            }
        }
    }
}
#endif