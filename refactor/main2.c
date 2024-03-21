#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "datasetReader.h"
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
    float avgUser1ratings = sumUser1ratings / numMoviesInCommon;
    float avgUser2ratings = sumUser2ratings / numMoviesInCommon;

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


void computeMostSimilarUsers(int n, Dataset* datasetPtr, int userId, int* arrayTopUsersIds, float* arrayTopUserScores,
                            float (*functionComputeSimilarity)(User*, User*)) {
    int numAddedMovies = 0;
    User* user = getUserById(datasetPtr, userId);
    for (int i = 0; i < datasetPtr->numUsers; i++) {
        User* newUser = datasetPtr->arrayUsers[i];
        if (newUser->id == userId) continue;
        float score = functionComputeSimilarity(user, newUser);
        if (!isnan(score)) {
            int pos = insertInFloatSortedArray(arrayTopUserScores, numAddedMovies, score, n);
            if (pos != -1) {
                insertInIntArray(arrayTopUsersIds, n, pos, newUser->id);
                if (numAddedMovies != n) numAddedMovies++;
            }
        }
    }
}

float predictUserRatingForMovie(Dataset* datasetPtr, int userId, int movieId,
                                float (*functionComputeSimilarity)(User*, User*)) {
    int S = 25;
    float arrayTopUsersScores[S];
    int arrayTopUsersIds[S];
    int numAddedUsers = 0;
    User* user = getUserById(datasetPtr, userId);
    for (int i = 0; i < datasetPtr->numUsers; i++) {
        User* newUser = datasetPtr->arrayUsers[i];
        int newUserId = newUser->id;
        if (newUserId == userId) continue;
        int rating = getValueFromHashMap(newUser->mapMovieIdRating, movieId);
        if (rating == -1) continue;
        float score = functionComputeSimilarity(user, newUser);
        if (!isnan(score)) {
            int pos = insertInFloatSortedArray(arrayTopUsersScores, numAddedUsers, score, S);
            if (pos != -1) {
                insertInIntArray(arrayTopUsersIds, S, pos, newUserId);
                if (numAddedUsers < S) numAddedUsers++;
            }
        }
    }
    float predictNumerator = 0.0;
    float predictDenominator = 0.0;
    for (int i = 0; i < numAddedUsers; i++) {
        int similarUserId = arrayTopUsersIds[i];
        float similarUserScore = arrayTopUsersScores[i];
        if (similarUserScore < 0.4) break;
        User* similarUser = getUserById(datasetPtr, similarUserId);
        float similarUserAvgRating = similarUser->averageRating;
        int rating = getValueFromHashMap(similarUser->mapMovieIdRating, movieId);
        predictNumerator += (similarUserScore*(rating-similarUserAvgRating));
        predictDenominator += similarUserScore;
    }
    if (predictDenominator == 0.0) return 0.75*user->averageRating;
    float predict = user->averageRating + (predictNumerator/predictDenominator);
    if (predict > 5.0) return 5.0;
    return predict;
}

void computeBestMoviesForUser(int n, int* arrayBestMoviesIds, float* arrayBestMoviesScores, Dataset* datasetPtr,
                            int userId, float (*functionComputeSimilarity)(User*, User*)) {
    int arrayMostSimilarUsersIds[25];
    float arrayMostSimilarUsersScores[25];
    computeMostSimilarUsers(25, datasetPtr, userId, arrayMostSimilarUsersIds, arrayMostSimilarUsersScores, computePearsonCorrelation);
    int numAddedMovies = 0;
    User* user = getUserById(datasetPtr, userId);
    for (int i = 0; i < 25; i++) {
        int similarUserId = arrayMostSimilarUsersIds[i];
        User* similarUser = getUserById(datasetPtr, similarUserId);
        for (int j = 0; j < similarUser->numRatings; j++) {
            int movieId = similarUser->arrayMoviesIds[j];
            if (isInIntArray(arrayBestMoviesIds, movieId, numAddedMovies))
                continue;
            if (getValueFromHashMap(user->mapMovieIdRating, movieId) != -1)
                continue;
            float score = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
            int pos = insertInFloatSortedArray(arrayBestMoviesScores, numAddedMovies, score, n);
            if (pos != -1) {
                insertInIntArray(arrayBestMoviesIds, numAddedMovies, pos, movieId);
                if (numAddedMovies < n) numAddedMovies++;
            }
        }
    }
}

void computeGroupRecommendations(int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                                float (*aggregationFunction)(float*, int), int* arrayMoviesIdsOutput, float* arrayMoviesScores) {
    int S = 10;
    int* arrayOfArraysMoviesSuggestionsForUser[groupSize];
    for (int i = 0; i < groupSize; i++) {
        int* arrayBestMoviesIds = (int*)malloc(S*sizeof(int));
        float arrayBestMoviesScores[S];
        computeBestMoviesForUser(S, arrayBestMoviesIds, arrayBestMoviesScores, datasetPtr, arrayGroupIds[i], functionComputeSimilarity);
        arrayOfArraysMoviesSuggestionsForUser[i] = arrayBestMoviesIds;
    }
    int arrayCandidateMoviesIds[S*groupSize];
    float* array_of_arrays_movies_scores_for_all_users[S*groupSize];
    int i = 0;
    for (int j = 0; j < groupSize; j++) {
        int* array_best_movies_ids = arrayOfArraysMoviesSuggestionsForUser[j];
        for (int k = 0; k < S; k++) {
            int movie_id = array_best_movies_ids[k];
            if (isInIntArray(arrayCandidateMoviesIds, movie_id, i)) continue;
            arrayCandidateMoviesIds[i] = movie_id;
            float* array_movies_scores = (float*)malloc(sizeof(float)*groupSize);
            for (int l = 0; l < groupSize; l++) {
                int user_id = arrayGroupIds[l];
                User* user = getUserById(datasetPtr, user_id);
                int rating = getValueFromHashMap(user->mapMovieIdRating, movie_id);
                if (rating == -1) {
                    float predict = predictUserRatingForMovie(datasetPtr, user_id, movie_id, functionComputeSimilarity);
                    array_movies_scores[l] = predict;
                }
                else {
                    array_movies_scores[l] = (float)rating;
                }
            }
            array_of_arrays_movies_scores_for_all_users[i] = array_movies_scores;
            i++;
        }
        free(array_best_movies_ids);
    }
    float array_movies_aggregated_scores[S*groupSize];
    for (int j = 0; j < i; j++) {
        float aggregated_score = aggregationFunction(array_of_arrays_movies_scores_for_all_users[j], groupSize);
        free(array_of_arrays_movies_scores_for_all_users[j]);
        array_movies_aggregated_scores[j] = aggregated_score;
    }
    for (int j = 0; j < 10; j++) {
        int pos = find_max_position_in_float_array(array_movies_aggregated_scores, i);
        arrayMoviesIdsOutput[j] = arrayCandidateMoviesIds[pos];
        arrayMoviesScores[j] = array_movies_aggregated_scores[pos];
        array_movies_aggregated_scores[pos] = 0.0;
    }
}


#define N 10

int main() {
    time_t start, end;
    double elapsed;
    start = clock();

    Dataset* datasetPtr = readCsv("ratings.csv");
    printf("csv number of rows: [%d]\n", datasetPtr->numUsers);
    float similarity = computeSimilarityBetweenUsersById(datasetPtr, 43, 500, computePearsonCorrelation);

    printf("%f\n", similarity);

    int arrayMostSimilarUsers[10];
    float arrayMostSimilarUsersScores[10];
    computeMostSimilarUsers(10, datasetPtr, 500, arrayMostSimilarUsers, arrayMostSimilarUsersScores, computePearsonCorrelation);
    printIntArray(arrayMostSimilarUsers, 10);
    printFloatArray(arrayMostSimilarUsersScores, 10);

    int arrayBestMoviesIds[10];
    float arrayBestMoviesRatings[10];
    computeBestMoviesForUser(10, arrayBestMoviesIds, arrayBestMoviesRatings, datasetPtr, 500, computePearsonCorrelation);
    printIntArray(arrayBestMoviesIds, 10);
    printFloatArray(arrayBestMoviesRatings, 10);

    float score = predictUserRatingForMovie(datasetPtr, 500, 430, computePearsonCorrelation);
    printf("%f\n\n", score);

    int groupSize = 3;
    int group[3] = {10, 37, 500};
    computeGroupRecommendations(groupSize, group, datasetPtr, computePearsonCorrelation, averageOfFloatArray, arrayBestMoviesIds, arrayBestMoviesRatings);
    printIntArray(arrayBestMoviesIds, 10);
    printFloatArray(arrayBestMoviesRatings, 10);

    freeDataset(datasetPtr);

    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\ntotal cpu time: [%f sec]\n", elapsed);
    return 0;
}