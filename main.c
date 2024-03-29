#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "datasetReader.h"
#include "project.h"
#include "utils.h"

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
    float* arrayOfArraysMoviesScoresForAllUsers[S*groupSize];
    int i = 0;
    for (int j = 0; j < groupSize; j++) {
        int* arrayBestMoviesIds = arrayOfArraysMoviesSuggestionsForUser[j];
        for (int k = 0; k < S; k++) {
            int movieId = arrayBestMoviesIds[k];
            if (isInIntArray(arrayCandidateMoviesIds, movieId, i)) continue;
            arrayCandidateMoviesIds[i] = movieId;
            float* arrayMoviesScores = (float*)malloc(sizeof(float)*groupSize);
            for (int l = 0; l < groupSize; l++) {
                int userId = arrayGroupIds[l];
                User* user = getUserById(datasetPtr, userId);
                int rating = getValueFromHashMap(user->mapMovieIdRating, movieId);
                if (rating == -1) {
                    float predict = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
                    arrayMoviesScores[l] = predict;
                }
                else {
                    arrayMoviesScores[l] = (float)rating;
                }
            }
            arrayOfArraysMoviesScoresForAllUsers[i] = arrayMoviesScores;
            i++;
        }
        free(arrayBestMoviesIds);
    }
    float arrayMoviesAggregatedScores[S*groupSize];
    for (int j = 0; j < i; j++) {
        float aggregated_score = aggregationFunction(arrayOfArraysMoviesScoresForAllUsers[j], groupSize);
        free(arrayOfArraysMoviesScoresForAllUsers[j]);
        arrayMoviesAggregatedScores[j] = aggregated_score;
    }
    for (int j = 0; j < 10; j++) {
        int pos = findMaxPositionInFloatArray(arrayMoviesAggregatedScores, i);
        arrayMoviesIdsOutput[j] = arrayCandidateMoviesIds[pos];
        arrayMoviesScores[j] = arrayMoviesAggregatedScores[pos];
        arrayMoviesAggregatedScores[pos] = 0.0;
    }
}

void computeGroupRecommendationsAlpha(int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                                float alpha, int* arrayMoviesIdsOutput, float* arrayMoviesScores) {
    int S = 10;
    int* arrayOfArraysMoviesSuggestionsForUser[groupSize];
    for (int i = 0; i < groupSize; i++) {
        int* arrayBestMoviesIds = (int*)malloc(S*sizeof(int));
        float arrayBestMoviesScores[S];
        computeBestMoviesForUser(S, arrayBestMoviesIds, arrayBestMoviesScores, datasetPtr, arrayGroupIds[i], functionComputeSimilarity);
        arrayOfArraysMoviesSuggestionsForUser[i] = arrayBestMoviesIds;
    }
    int arrayCandidateMoviesIds[S*groupSize];
    float* arrayOfArraysMoviesScoresForAllUsers[S*groupSize];
    int i = 0;
    for (int j = 0; j < groupSize; j++) {
        int* arrayBestMoviesIds = arrayOfArraysMoviesSuggestionsForUser[j];
        for (int k = 0; k < S; k++) {
            int movieId = arrayBestMoviesIds[k];
            if (isInIntArray(arrayCandidateMoviesIds, movieId, i)) continue;
            arrayCandidateMoviesIds[i] = movieId;
            float* arrayMoviesScores = (float*)malloc(sizeof(float)*groupSize);
            for (int l = 0; l < groupSize; l++) {
                int userId = arrayGroupIds[l];
                User* user = getUserById(datasetPtr, userId);
                int rating = getValueFromHashMap(user->mapMovieIdRating, movieId);
                if (rating == -1) {
                    float predict = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
                    arrayMoviesScores[l] = predict;
                }
                else {
                    arrayMoviesScores[l] = (float)rating;
                }
            }
            arrayOfArraysMoviesScoresForAllUsers[i] = arrayMoviesScores;
            i++;
        }
        free(arrayBestMoviesIds);
    }
    float arrayMoviesAggregatedScores[S*groupSize];
    for (int j = 0; j < i; j++) {
        float aggregated_score = (1-alpha)*averageOfFloatArray(arrayOfArraysMoviesScoresForAllUsers[j], groupSize) + alpha*findMinInFloatArray(arrayOfArraysMoviesScoresForAllUsers[j], groupSize);
        free(arrayOfArraysMoviesScoresForAllUsers[j]);
        arrayMoviesAggregatedScores[j] = aggregated_score;
    }
    for (int j = 0; j < 10; j++) {
        int pos = findMaxPositionInFloatArray(arrayMoviesAggregatedScores, i);
        arrayMoviesIdsOutput[j] = arrayCandidateMoviesIds[pos];
        arrayMoviesScores[j] = arrayMoviesAggregatedScores[pos];
        arrayMoviesAggregatedScores[pos] = 0.0;
    }
}

void computeGroupRecommendationsWithDisagreement(int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                                    float (*aggregationFunction)(float*, int), int* arrayMoviesIdsOutput, float* arrayMoviesScoresOutput) {
    int arrayMoviesIds[10];
    float arrayAggregatedScores[10];
    computeGroupRecommendations(groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, aggregationFunction, arrayMoviesIds, arrayAggregatedScores);
    int* arrayOfArraysUserMoviesOrderPreference[groupSize];
    for (int i = 0; i < groupSize; i++) {
        int userId = arrayGroupIds[i];
        int* arrayUserMoviesOrderPreference = (int*)malloc(sizeof(int)*10);
        float arrayUserMoviesScores[10];
        for (int j = 0; j < 10; j++) {
            int movieId = arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
            int pos = insertInFloatSortedArray(arrayUserMoviesScores, j, predict, 10);
            insertInIntArray(arrayUserMoviesOrderPreference, 10, pos, movieId);
        }
        arrayOfArraysUserMoviesOrderPreference[i] = arrayUserMoviesOrderPreference;
    }
    int arrayMoviesIdsCopy[10];
    for (int i = 0; i < 10; i++)
        arrayMoviesIdsCopy[i] = arrayMoviesIds[i];
    for (int i = 0; i < 10; i++) {
        int j = 0;
        while (arrayMoviesIdsCopy[j] == -1) j++;
        int candidateMovieId = arrayMoviesIdsCopy[j];
        int candidateMovieIdTotalDisagreement = 0;
        for (int k = 0; k < groupSize; k++) {
            int pos = posInIntArray(arrayOfArraysUserMoviesOrderPreference[k], 10, candidateMovieId);
            candidateMovieIdTotalDisagreement += abs(i-pos);
        }
        for (int k = j+1; k < 10; k++) {
            int movieId = arrayMoviesIdsCopy[k];
            if (movieId == -1) continue;
            int disagreement = 0;
            for (int l = 0; l < groupSize; l++) {
                int pos = posInIntArray(arrayOfArraysUserMoviesOrderPreference[l], 10, candidateMovieId);
                disagreement += abs(i-pos);
            }
            if (disagreement < candidateMovieIdTotalDisagreement) {
                candidateMovieIdTotalDisagreement = disagreement;
                candidateMovieId = movieId;
            }
        }
        arrayMoviesIdsOutput[i] = candidateMovieId;
        int pos = posInIntArray(arrayMoviesIdsCopy, 10, candidateMovieId);
        arrayMoviesIdsCopy[pos] = -1;
    }
    for (int i = 0; i < 10; i++) {
        int movieId = arrayMoviesIdsOutput[i];
        int pos = posInIntArray(arrayMoviesIds, 10, movieId);
        arrayMoviesScoresOutput[i] = arrayAggregatedScores[pos];
    }
}

void computeGroupRecommendationsSequential(int n, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                    float (*aggregationFunction)(float*, int),
                    int numIterations, int** arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, int* arrayNumSuggestedMoviesAtIteration) {
    if (numIterations == 0) {
        float ratings[10];
        int movieIds[10];
        computeGroupRecommendations(groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, aggregationFunction, movieIds, ratings);
        int* movieIdsOutput = (int*)malloc(n*sizeof(int));
        for (int i = 0; i < n; i++) {
            int pos = findMaxPositionInFloatArray(ratings, 10);
            movieIdsOutput[i] = movieIds[pos];
            ratings[pos] = 0;
        }
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[0] = movieIdsOutput;
        arrayNumSuggestedMoviesAtIteration[0] = n;
        return;
    }
    float arraySatisfactionPerUser[groupSize];
    for (int i = 0; i < groupSize; i++) {
        User* userPtr = getUserById(datasetPtr, arrayGroupIds[i]);
        float arrayMoviesRatingsCopy[userPtr->numRatings];
        for (int j = 0; j < userPtr->numRatings; j++)
            arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
        quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
        float overallSatisfaction = 0.0;
        for (int j = 0; j < numIterations; j++) {
            float numerator = 0.0;
            float denominator = 0.0;
            int* arrayMoviesIdsAtIteration = arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[j];
            int numMovies = arrayNumSuggestedMoviesAtIteration[j];
            for (int k = 0; k < numMovies; k++) {
                numerator += predictUserRatingForMovie(datasetPtr, arrayGroupIds[i], arrayMoviesIdsAtIteration[k], functionComputeSimilarity);
                denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-k];
            }
            overallSatisfaction += (numerator/denominator);
        }
        overallSatisfaction = overallSatisfaction/numIterations;
        arraySatisfactionPerUser[i] = overallSatisfaction;
    }
    printFloatArray(arraySatisfactionPerUser, groupSize);
    int maxPos = findMaxPositionInFloatArray(arraySatisfactionPerUser, groupSize);
    float min = findMinInFloatArray(arraySatisfactionPerUser, groupSize);
    float alpha = arraySatisfactionPerUser[maxPos]-min;
    printf("%f\n", alpha);
    int arrayMoviesIdsOutput[10];
    float arrayMoviesScores[10];
    computeGroupRecommendationsAlpha(groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity,
                                alpha, arrayMoviesIdsOutput, arrayMoviesScores);
    int z = 0;
    arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations] = (int*)malloc(n*sizeof(int));
    arrayNumSuggestedMoviesAtIteration[numIterations] = n;
    for (int i = 0; i < 10; i++) {
        int movieId = arrayMoviesIdsOutput[i];
        if (z == n) break;
        int alreadySuggested = 0;
        for (int j = 0; j < numIterations; j++) {
            int* moviesIds = arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[j];
            for (int k = 0; k < arrayNumSuggestedMoviesAtIteration[j]; k++) {
                if (isInIntArray(moviesIds, movieId, arrayNumSuggestedMoviesAtIteration[j])) {
                    alreadySuggested = 1;
                    break;
                }
            }
        }
        if (alreadySuggested) continue;
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations][z] = movieId;
        z++;
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

    computeGroupRecommendationsWithDisagreement(groupSize, group, datasetPtr, computePearsonCorrelation, averageOfFloatArray, arrayBestMoviesIds, arrayBestMoviesRatings);
    printIntArray(arrayBestMoviesIds, 10);
    printFloatArray(arrayBestMoviesRatings, 10);

    int* arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[3];
    int arrayNumSuggestedMoviesAtIteration[3];
    computeGroupRecommendationsSequential(3, groupSize, group, datasetPtr, computePearsonCorrelation, averageOfFloatArray, 0, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[0], arrayNumSuggestedMoviesAtIteration[0]);
    computeGroupRecommendationsSequential(3, groupSize, group, datasetPtr, computePearsonCorrelation, averageOfFloatArray, 1, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[1], arrayNumSuggestedMoviesAtIteration[0]);
    computeGroupRecommendationsSequential(3, groupSize, group, datasetPtr, computePearsonCorrelation, averageOfFloatArray, 2, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[2], arrayNumSuggestedMoviesAtIteration[0]);
    
    freeDataset(datasetPtr);

    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\ntotal cpu time: [%f sec]\n", elapsed);
    return 0;
}