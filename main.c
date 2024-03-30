#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "datasetReader.h"
#include "project.h"
#include "utils.h"

#define N 10

int main() {
    time_t start, end;
    double elapsed;

    // reading csv
    start = clock();
    Dataset* datasetPtr = readCsv("ratings.csv");
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("time to read and parse csv: [%f sec]\n", elapsed);
    printf("number of users: [%d]\n", datasetPtr->numUsers);

    // computing similarity between 2 users
    start = clock();
    int user0 = 43;
    int user1 = 500;
    float similarity = computeSimilarityBetweenUsersById(datasetPtr, user0, user1, computePearsonCorrelation);
    end = clock();
    printf("\nsimilarity between user [%d] and user [%d]: [%f]\n", user0, user1, similarity);
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("time to compute similarity between users: [%f sec]\n", elapsed);
        
    // computing most similar users
    start = clock();
    int numMostSimilarUsers = 10;
    int arrayMostSimilarUsers[numMostSimilarUsers];
    float arrayMostSimilarUsersScores[numMostSimilarUsers];
    computeMostSimilarUsers(numMostSimilarUsers, datasetPtr, user1, arrayMostSimilarUsers, arrayMostSimilarUsersScores, computePearsonCorrelation);
    end = clock();
    printf("\nmost similar users to user [%d]:\n", user1);
    printIntArray(arrayMostSimilarUsers, numMostSimilarUsers);
    printf("similarity score for each user:\n");
    printFloatArray(arrayMostSimilarUsersScores, numMostSimilarUsers);
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("time to compute %d most similar users: [%f sec]\n", numMostSimilarUsers, elapsed);

    // computing movies suggestions
    start = clock();
    int numMoviesSuggestions = 10;
    int arrayBestMoviesIds[numMoviesSuggestions];
    float arrayBestMoviesRatings[numMoviesSuggestions];
    computeBestMoviesForUser(numMoviesSuggestions, arrayBestMoviesIds, arrayBestMoviesRatings, datasetPtr, user1, computePearsonCorrelation);
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\nsuggested movies ids for user [%d]:\n", user1);
    printIntArray(arrayBestMoviesIds, numMoviesSuggestions);
    printf("predicted score for each movie:\n");
    printFloatArray(arrayBestMoviesRatings, numMoviesSuggestions);
    printf("time to compute %d movie suggestions: [%f sec]\n", numMoviesSuggestions, elapsed);

    // computing prediction for movie for user
    start = clock();
    int movieId = 430;
    float score = predictUserRatingForMovie(datasetPtr, user1, movieId, computePearsonCorrelation);
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\npredicted user [%d] rating for movie [%d]: [%f]\n", user1, movieId, score);
    printf("time to compute prediction for movie: [%f sec]\n", elapsed);

    // computing group suggestions
    start = clock();
    int groupSize = 3;
    int groupIds[3] = {10, 37, 500};
    int numGroupSuggestions = 10;
    int arrayGroupMoviesSuggestionsIds[numGroupSuggestions];
    float arrayGroupMoviesSuggestionsScores[numGroupSuggestions];
    computeGroupRecommendationsAverage(numGroupSuggestions, groupSize, groupIds, datasetPtr, computePearsonCorrelation, arrayGroupMoviesSuggestionsIds, arrayGroupMoviesSuggestionsScores);
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\ngroup: ");
    printIntArray(groupIds, groupSize);
    printf("movies suggestions for group:\n");
    printIntArray(arrayGroupMoviesSuggestionsIds, numGroupSuggestions);
    printf("aggregated scores for each movie (average):\n");
    printFloatArray(arrayGroupMoviesSuggestionsScores, numGroupSuggestions);
    printf("time to compute group recommentations: [%f sec]\n", elapsed);

    // computing group suggestions with disagreement
    start = clock();
    computeGroupRecommendationsWithDisagreement(numGroupSuggestions, groupSize, groupIds, datasetPtr, computePearsonCorrelation, arrayGroupMoviesSuggestionsIds, arrayGroupMoviesSuggestionsScores);
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\nmovies suggestions for group (with disagreement):\n");
    printIntArray(arrayGroupMoviesSuggestionsIds, numGroupSuggestions);
    printf("aggregated scores for each movie (average):\n");
    printFloatArray(arrayGroupMoviesSuggestionsScores, numGroupSuggestions);
    printf("time to compute group recommentations (with disagreement): [%f sec]\n", elapsed);

    // computing group suggestions sequentially
    start = clock();
    int totalIterations = 3;
    int* arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[totalIterations];
    int arrayNumSuggestedMoviesAtIteration[totalIterations];
    int numSuggestions = 3;
    printf("\nsequential suggestions for group:\n");
    computeGroupRecommendationsSequential(numSuggestions, groupSize, groupIds, datasetPtr, computePearsonCorrelation, 0, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    computeGroupRecommendationsSequential(numSuggestions, groupSize, groupIds, datasetPtr, computePearsonCorrelation, 1, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    computeGroupRecommendationsSequential(numSuggestions+1, groupSize, groupIds, datasetPtr, computePearsonCorrelation, 2, arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration, arrayNumSuggestedMoviesAtIteration);
    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("suggestions for group at iteration [%d]\n", 0);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[0], arrayNumSuggestedMoviesAtIteration[0]);
    printf("suggestions for group at iteration [%d]\n", 1);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[1], arrayNumSuggestedMoviesAtIteration[0]);
    printf("suggestions for group at iteration [%d]\n", 2);
    printIntArray(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[2], arrayNumSuggestedMoviesAtIteration[0]);
    printf("time to compute group recommentations sequentially (3 iterations): [%f sec]\n", elapsed);
    
    // freeing heap
    freeDataset(datasetPtr);
    for (int i = 0; i < 3; i++)
        free(arrayOfArraysPreviouslySuggestedMoviesIdsAtIteration[i]);
    return 0;
}