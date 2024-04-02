#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

float computeJaccardIndex(User* user1ptr, User* user2ptr) {
    int numerator = 0;
    int denominator = user1ptr->numRatings;
    HashMap* user2ratingsMap = user2ptr->mapMovieIdRating;
    int* arrayUser1moviesIds = user1ptr->arrayMoviesIds;
    for (int i = 0; i < user1ptr->numRatings; i++)
        if (getValueFromHashMap(user2ratingsMap, arrayUser1moviesIds[i]) != -1)
            numerator++;
    HashMap* user1ratingsMap = user1ptr->mapMovieIdRating;
    int* arrayUser2moviesIds = user2ptr->arrayMoviesIds;
    for (int i = 0; i < user2ptr->numRatings; i++)
        if (getValueFromHashMap(user1ratingsMap, arrayUser2moviesIds[i]) == -1)
            denominator++;
    return 2*(0.5-numerator/(float)denominator);
}

float computeSimilarityBetweenUsersById(Dataset* datasetPtr, int user1id, int user2id, 
                                        float (*functionComputeSimilarity)(User*, User*)) {
    return functionComputeSimilarity(getUserById(datasetPtr, user1id), getUserById(datasetPtr, user2id));
}

void computeMostSimilarUsers(int numSimilarUsers, Dataset* datasetPtr, int userId, int* arrayTopUsersIds, float* arrayTopUserScores,
                            float (*functionComputeSimilarity)(User*, User*)) {
    int numAddedUsers = 0;
    User* userPtr = getUserById(datasetPtr, userId);
    User** arrayUsers = datasetPtr->arrayUsers;
    int numUsers = datasetPtr->numUsers;
    for (int i = 0; i < numUsers; i++) {
        User* newUserPtr = arrayUsers[i];
        if (newUserPtr->id == userId) continue;
        float score = functionComputeSimilarity(userPtr, newUserPtr);
        if (!isnan(score)) {
            int pos = insertInFloatSortedArray(arrayTopUserScores, numAddedUsers, score, numSimilarUsers);
            if (pos != -1) {
                insertInIntArray(arrayTopUsersIds, numSimilarUsers, pos, newUserPtr->id);
                if (numAddedUsers != numSimilarUsers) numAddedUsers++;
            }
        }
    }
}

# define MOST_SIMILAR_USERS_NUM 25
float predictUserRatingForMovie(Dataset* datasetPtr, int userId, int movieId,
                                float (*functionComputeSimilarity)(User*, User*)) {
    float arrayTopUsersScores[MOST_SIMILAR_USERS_NUM];
    int arrayTopUsersIds[MOST_SIMILAR_USERS_NUM];
    int numAddedUsers = 0;
    User* userPtr = getUserById(datasetPtr, userId);
    int numUsers = datasetPtr->numUsers;
    User** arrayUsers = datasetPtr->arrayUsers;
    for (int i = 0; i < numUsers; i++) {
        User* newUserPtr = arrayUsers[i];
        int newUserId = newUserPtr->id;
        if (newUserId == userId) continue;
        int rating = getValueFromHashMap(newUserPtr->mapMovieIdRating, movieId);
        if (rating == -1) continue; // if new user didnt rate movie, skip him
        float score = functionComputeSimilarity(userPtr, newUserPtr);
        if (!isnan(score)) {
            int pos = insertInFloatSortedArray(arrayTopUsersScores, numAddedUsers, score, MOST_SIMILAR_USERS_NUM);
            if (pos != -1) {
                insertInIntArray(arrayTopUsersIds, MOST_SIMILAR_USERS_NUM, pos, newUserId);
                if (numAddedUsers < MOST_SIMILAR_USERS_NUM) numAddedUsers++;
            }
        }
    }
    float predictNumerator = 0.0;
    float predictDenominator = 0.0;
    for (int i = 0; i < numAddedUsers; i++) {
        int similarUserId = arrayTopUsersIds[i];
        float similarUserScore = arrayTopUsersScores[i];
        if (similarUserScore < 0.4) break;
        User* similarUserPtr = getUserById(datasetPtr, similarUserId);
        int rating = getValueFromHashMap(similarUserPtr->mapMovieIdRating, movieId);
        predictNumerator += (similarUserScore*(rating-similarUserPtr->averageRating));
        predictDenominator += similarUserScore;
    }
    if (predictDenominator == 0.0) return 0.75*userPtr->averageRating;
    float predict = userPtr->averageRating + (predictNumerator/predictDenominator);
    if (predict > 5.0) return 5.0;
    return predict;
}

void computeBestMoviesForUser(int numSuggestions, int* arrayBestMoviesIds, float* arrayBestMoviesScores, Dataset* datasetPtr,
                            int userId, float (*functionComputeSimilarity)(User*, User*)) {
    int arrayMostSimilarUsersIds[MOST_SIMILAR_USERS_NUM];
    float arrayMostSimilarUsersScores[MOST_SIMILAR_USERS_NUM];
    computeMostSimilarUsers(MOST_SIMILAR_USERS_NUM, datasetPtr, userId, arrayMostSimilarUsersIds, arrayMostSimilarUsersScores, computePearsonCorrelation);
    int numAddedMovies = 0;
    User* userPtr = getUserById(datasetPtr, userId);
    for (int i = 0; i < MOST_SIMILAR_USERS_NUM; i++) {
        int similarUserId = arrayMostSimilarUsersIds[i];
        User* similarUserPtr = getUserById(datasetPtr, similarUserId);
        int numRatings = similarUserPtr->numRatings;
        int* arrayMoviesIds = similarUserPtr->arrayMoviesIds;
        for (int j = 0; j < numRatings; j++) {
            int movieId = arrayMoviesIds[j];
            if (isInIntArray(arrayBestMoviesIds, movieId, numAddedMovies))
                continue;
            if (getValueFromHashMap(userPtr->mapMovieIdRating, movieId) != -1) // if user already rated movie, skip it
                continue;
            float score = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
            int pos = insertInFloatSortedArray(arrayBestMoviesScores, numAddedMovies, score, numSuggestions);
            if (pos != -1) {
                insertInIntArray(arrayBestMoviesIds, numAddedMovies, pos, movieId);
                if (numAddedMovies < numSuggestions) numAddedMovies++;
            }
        }
    }
}

# define NUM_MOVIES_SUGGESTION_PER_USER 15
void computeGroupRecommendationsAlpha(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                            float alpha, int* arrayMoviesIdsOutput, float* arrayMoviesScores) {
    int* arrayOfArraysMoviesSuggestionsForUser[groupSize];
    for (int i = 0; i < groupSize; i++) {
        int* arrayBestMoviesIds = (int*)malloc(NUM_MOVIES_SUGGESTION_PER_USER*sizeof(int));
        float arrayBestMoviesScores[NUM_MOVIES_SUGGESTION_PER_USER]; // not actually used
        computeBestMoviesForUser(NUM_MOVIES_SUGGESTION_PER_USER, arrayBestMoviesIds, arrayBestMoviesScores, datasetPtr, arrayGroupIds[i], functionComputeSimilarity);
        arrayOfArraysMoviesSuggestionsForUser[i] = arrayBestMoviesIds;
    }
    int arrayCandidateMoviesIds[NUM_MOVIES_SUGGESTION_PER_USER*groupSize];
    float* arrayOfArraysMoviesScoresForAllUsers[NUM_MOVIES_SUGGESTION_PER_USER*groupSize];
    int numCandidateMovies = 0;
    for (int i = 0; i < groupSize; i++) {
        int* arrayBestMoviesIds = arrayOfArraysMoviesSuggestionsForUser[i];
        for (int j = 0; j < NUM_MOVIES_SUGGESTION_PER_USER; j++) {
            int movieId = arrayBestMoviesIds[j];
            if (isInIntArray(arrayCandidateMoviesIds, movieId, numCandidateMovies)) continue;
            arrayCandidateMoviesIds[numCandidateMovies] = movieId;
            float* arrayMoviesScores = (float*)malloc(sizeof(float)*groupSize);
            for (int k = 0; k < groupSize; k++) {
                int userId = arrayGroupIds[k];
                User* userPtr = getUserById(datasetPtr, userId);
                float rating = getValueFromHashMap(userPtr->mapMovieIdRating, movieId);
                if (rating == -1)
                    rating = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
                arrayMoviesScores[k] = rating;
            }
            arrayOfArraysMoviesScoresForAllUsers[numCandidateMovies] = arrayMoviesScores;
            numCandidateMovies++;
        }
        free(arrayBestMoviesIds);
    }
    float arrayMoviesAggregatedScores[NUM_MOVIES_SUGGESTION_PER_USER*groupSize];
    for (int i = 0; i < numCandidateMovies; i++) {
        float* arrayScoresForAllUsers = arrayOfArraysMoviesScoresForAllUsers[i];
        float aggregatedScore = (1-alpha)*averageOfFloatArray(arrayScoresForAllUsers, groupSize) + alpha*findMinInFloatArray(arrayScoresForAllUsers, groupSize);
        free(arrayScoresForAllUsers);
        arrayMoviesAggregatedScores[i] = aggregatedScore;
    }
    for (int j = 0; j < numSuggestions; j++) {
        int pos = findMaxPositionInFloatArray(arrayMoviesAggregatedScores, numCandidateMovies);
        arrayMoviesIdsOutput[j] = arrayCandidateMoviesIds[pos];
        arrayMoviesScores[j] = arrayMoviesAggregatedScores[pos];
        arrayMoviesAggregatedScores[pos] = 0.0;
    }
}

void computeGroupRecommendationsAverage(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                            int* arrayMoviesIdsOutput, float* arrayMoviesScores) {
    computeGroupRecommendationsAlpha(numSuggestions, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, 0, arrayMoviesIdsOutput, arrayMoviesScores);    
}

void computeGroupRecommendationsLeastMisery(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                            int* arrayMoviesIdsOutput, float* arrayMoviesScores) {
    computeGroupRecommendationsAlpha(numSuggestions, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, 1, arrayMoviesIdsOutput, arrayMoviesScores);    
}

void computeGroupRecommendationsWithDisagreement(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                                int* arrayMoviesIdsOutput, float* arrayMoviesScoresOutput) {
    int arrayMoviesIds[numSuggestions];
    float arrayAggregatedScores[numSuggestions];
    computeGroupRecommendationsAverage(numSuggestions, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, arrayMoviesIds, arrayAggregatedScores);
    int* arrayOfArraysUserMoviesOrderPreference[groupSize];
    for (int i = 0; i < groupSize; i++) {
        int userId = arrayGroupIds[i];
        int* arrayUserMoviesOrderPreference = (int*)malloc(sizeof(int)*numSuggestions);
        float arrayUserMoviesScores[numSuggestions];
        for (int j = 0; j < numSuggestions; j++) {
            int movieId = arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userId, movieId, functionComputeSimilarity);
            int pos = insertInFloatSortedArray(arrayUserMoviesScores, j, predict, numSuggestions);
            insertInIntArray(arrayUserMoviesOrderPreference, numSuggestions, pos, movieId);
        }
        arrayOfArraysUserMoviesOrderPreference[i] = arrayUserMoviesOrderPreference;
    }
    int arrayMoviesIdsCopy[numSuggestions];
    for (int i = 0; i < numSuggestions; i++)
        arrayMoviesIdsCopy[i] = arrayMoviesIds[i];
    for (int i = 0; i < numSuggestions; i++) {
        int j = 0;
        while (arrayMoviesIdsCopy[j] == -1) j++;
        int candidateMovieId = arrayMoviesIdsCopy[j];
        int candidateMovieIdTotalDisagreement = 0;
        for (int k = 0; k < groupSize; k++) {
            int pos = posInIntArray(arrayOfArraysUserMoviesOrderPreference[k], numSuggestions, candidateMovieId);
            candidateMovieIdTotalDisagreement += abs(i-pos);
        }
        for (int k = j+1; k < numSuggestions; k++) {
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
        int pos = posInIntArray(arrayMoviesIdsCopy, numSuggestions, candidateMovieId);
        arrayMoviesIdsCopy[pos] = -1;
    }
    for (int i = 0; i < numSuggestions; i++) {
        int movieId = arrayMoviesIdsOutput[i];
        int pos = posInIntArray(arrayMoviesIds, numSuggestions, movieId);
        arrayMoviesScoresOutput[i] = arrayAggregatedScores[pos];
    }
    for (int i = 0; i < groupSize; i++)
        free(arrayOfArraysUserMoviesOrderPreference[i]);
}

float alphaMaxMinusMin(float* arraySatisfactionPerUser, int groupSize) {
    int maxPos = findMaxPositionInFloatArray(arraySatisfactionPerUser, groupSize);
    float min = findMinInFloatArray(arraySatisfactionPerUser, groupSize);
    float alpha = arraySatisfactionPerUser[maxPos]-min;
    return alpha;
}

float alphaStandardDeviation(float* arraySatisfactionPerUser, int groupSize) {
    float numerator = 0.0;
    float average = 0.0;
    for (int i = 0; i < groupSize; i++)
        average += arraySatisfactionPerUser[i];
    average = average / groupSize;
    for (int i = 0; i < groupSize; i++)
        numerator += pow(arraySatisfactionPerUser[i]-average, 2);
    return sqrt(numerator/(groupSize-1));
}

#define MAX_GROUP_SUGGESTIONS_NUM 25
void computeGroupRecommendationsSequential(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                    int numIterations, int** arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, int* arrayNumSuggestedMoviesAtIteration,
                    float (*functionComputeAlpha)(float*, int)) {
    if (numIterations == 0) {
        float ratings[numSuggestions];
        int* movieIds = (int*)malloc(numSuggestions*sizeof(int));
        computeGroupRecommendationsAverage(numSuggestions, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, movieIds, ratings);
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[0] = movieIds;
        arrayNumSuggestedMoviesAtIteration[0] = numSuggestions;
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
    printf("satisfaction per user at iteration [%d]:\n", numIterations);
    printFloatArray(arraySatisfactionPerUser, groupSize);
    float alpha = functionComputeAlpha(arraySatisfactionPerUser, groupSize);
    printf("alpha: [%f]\n", alpha);
    int arrayMoviesIdsOutput[MAX_GROUP_SUGGESTIONS_NUM];
    float arrayMoviesScores[MAX_GROUP_SUGGESTIONS_NUM];
    computeGroupRecommendationsAlpha(MAX_GROUP_SUGGESTIONS_NUM, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity,
                                alpha, arrayMoviesIdsOutput, arrayMoviesScores);
    int numAddedSuggestions = 0;
    arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations] = (int*)malloc(numSuggestions*sizeof(int));
    arrayNumSuggestedMoviesAtIteration[numIterations] = numSuggestions;
    for (int i = 0; i < MAX_GROUP_SUGGESTIONS_NUM; i++) {
        int movieId = arrayMoviesIdsOutput[i];
        if (numAddedSuggestions == numSuggestions) break;
        int alreadySuggestedInThePast = 0;
        for (int j = 0; j < numIterations; j++) {
            int* moviesIds = arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[j];
            int numMovies = arrayNumSuggestedMoviesAtIteration[j];
            for (int k = 0; k < numMovies; k++) {
                if (isInIntArray(moviesIds, movieId, arrayNumSuggestedMoviesAtIteration[j])) {
                    alreadySuggestedInThePast = 1;
                    break;
                }
            }
        }
        if (alreadySuggestedInThePast) continue;
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations][numAddedSuggestions] = movieId;
        numAddedSuggestions++;
    }    
}

// instead of the alpha defined before we use the standard deviation of satisfaction
void computeGroupRecommendationsSequentialSD(int numSuggestions, int groupSize, int* arrayGroupIds, Dataset* datasetPtr, float (*functionComputeSimilarity)(User*, User*),
                    int numIterations, int** arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, int* arrayNumSuggestedMoviesAtIteration) {
    if (numIterations == 0) {
        float ratings[numSuggestions];
        int* movieIds = (int*)malloc(numSuggestions*sizeof(int));
        computeGroupRecommendationsAverage(numSuggestions, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity, movieIds, ratings);
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[0] = movieIds;
        arrayNumSuggestedMoviesAtIteration[0] = numSuggestions;
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
    printf("satisfaction per user at iteration [%d]:\n", numIterations);
    printFloatArray(arraySatisfactionPerUser, groupSize);
    int maxPos = findMaxPositionInFloatArray(arraySatisfactionPerUser, groupSize);
    float min = findMinInFloatArray(arraySatisfactionPerUser, groupSize);
    float alpha = arraySatisfactionPerUser[maxPos]-min;
    printf("alpha: [%f]\n", alpha);
    int arrayMoviesIdsOutput[MAX_GROUP_SUGGESTIONS_NUM];
    float arrayMoviesScores[MAX_GROUP_SUGGESTIONS_NUM];
    computeGroupRecommendationsAlpha(MAX_GROUP_SUGGESTIONS_NUM, groupSize, arrayGroupIds, datasetPtr, functionComputeSimilarity,
                                alpha, arrayMoviesIdsOutput, arrayMoviesScores);
    int numAddedSuggestions = 0;
    arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations] = (int*)malloc(numSuggestions*sizeof(int));
    arrayNumSuggestedMoviesAtIteration[numIterations] = numSuggestions;
    for (int i = 0; i < MAX_GROUP_SUGGESTIONS_NUM; i++) {
        int movieId = arrayMoviesIdsOutput[i];
        if (numAddedSuggestions == numSuggestions) break;
        int alreadySuggestedInThePast = 0;
        for (int j = 0; j < numIterations; j++) {
            int* moviesIds = arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[j];
            int numMovies = arrayNumSuggestedMoviesAtIteration[j];
            for (int k = 0; k < numMovies; k++) {
                if (isInIntArray(moviesIds, movieId, arrayNumSuggestedMoviesAtIteration[j])) {
                    alreadySuggestedInThePast = 1;
                    break;
                }
            }
        }
        if (alreadySuggestedInThePast) continue;
        arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[numIterations][numAddedSuggestions] = movieId;
        numAddedSuggestions++;
    }    
}

#endif