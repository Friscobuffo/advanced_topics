#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "datasetReader.h"
#include "project.h"
#include "utils.h"

float multiplyJaccardPearson(User* user1ptr, User* user2ptr) {
    float pearson = computePearsonCorrelation(user1ptr, user2ptr);
    float jaccard = computeJaccardIndex(user1ptr, user2ptr);
    return pearson*jaccard;
}

float multiplyJaccardPearson2(User* user1ptr, User* user2ptr) {
    float pearson = computePearsonCorrelation(user1ptr, user2ptr);
    float jaccard = computeJaccardIndex(user1ptr, user2ptr);
    return pearson*pow(jaccard, 0.5);
}

void test1(Dataset* datasetPtr) {
    srandom(1000);
    float meanSquaredError = 0.0;
    int n = 0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        User* userPtr = getUserById(datasetPtr, random()%datasetPtr->numUsers);
        for (int j = 0; j < userPtr->numRatings; j++) {
            float rating = userPtr->arrayMoviesRatings[j];
            int movieId = userPtr->arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userPtr->id, movieId, computePearsonCorrelation);
            n++;
            meanSquaredError += pow(rating-predict, 2);
        }
    }
    meanSquaredError = meanSquaredError/n;
    printf("\nmean squared error (pearson coefficient): [%f]\n", meanSquaredError);
}

void test2(Dataset* datasetPtr) {
    srandom(1000);
    float meanSquaredError = 0.0;
    int n = 0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        User* userPtr = getUserById(datasetPtr, random()%datasetPtr->numUsers);
        for (int j = 0; j < userPtr->numRatings; j++) {
            float rating = userPtr->arrayMoviesRatings[j];
            int movieId = userPtr->arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userPtr->id, movieId, computeJaccardIndex);
            n++;
            meanSquaredError += pow(rating-predict, 2);
        }
    }
    meanSquaredError = meanSquaredError/n;
    printf("\nmean squared error (jaccard index): [%f]\n", meanSquaredError);
}

void test3(Dataset* datasetPtr) {
    srandom(1000);
    float meanSquaredError = 0.0;
    int n = 0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        User* userPtr = getUserById(datasetPtr, random()%datasetPtr->numUsers);
        for (int j = 0; j < userPtr->numRatings; j++) {
            float rating = userPtr->arrayMoviesRatings[j];
            int movieId = userPtr->arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userPtr->id, movieId, multiplyJaccardPearson);
            n++;
            meanSquaredError += pow(rating-predict, 2);
        }
    }
    meanSquaredError = meanSquaredError/n;
    printf("\nmean squared error (jaccard*pearson): [%f]\n", meanSquaredError);
}

void test4(Dataset* datasetPtr) {
    srandom(1000);
    float meanSquaredError = 0.0;
    int n = 0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        User* userPtr = getUserById(datasetPtr, random()%datasetPtr->numUsers);
        for (int j = 0; j < userPtr->numRatings; j++) {
            float rating = userPtr->arrayMoviesRatings[j];
            int movieId = userPtr->arrayMoviesIds[j];
            float predict = predictUserRatingForMovie(datasetPtr, userPtr->id, movieId, multiplyJaccardPearson2);
            n++;
            meanSquaredError += pow(rating-predict, 2);
        }
    }
    meanSquaredError = meanSquaredError/n;
    printf("\nmean squared error (pearson*pow(jaccard)): [%f]\n", meanSquaredError);
}

void test5(Dataset* datasetPtr) {
    srandom(1000);
    int numSuggestions = 10;
    int suggestions[numSuggestions];
    float scores[numSuggestions];
    float totalSatisfaction = 0.0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        int userId = random()%datasetPtr->numUsers;
        while (userId == 53) userId = random()%datasetPtr->numUsers;
        User* userPtr = getUserById(datasetPtr, userId);
        computeBestMoviesForUser(numSuggestions, suggestions, scores, datasetPtr, userPtr->id, computePearsonCorrelation);
        float arrayMoviesRatingsCopy[userPtr->numRatings];
        for (int j = 0; j < userPtr->numRatings; j++)
            arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
        quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
        float numerator = 0.0;
        float denominator = 0.0;
        for (int j = 0; j < numSuggestions; j++) {
            numerator += scores[j];
            denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-j];
        }
        float satisfaction = numerator/denominator;
        totalSatisfaction += satisfaction;
    }
    float averageSatisfaction = totalSatisfaction/50;
    printf("\naverage satisfaction (pearson coefficient): [%f]\n", averageSatisfaction);
}

void test6(Dataset* datasetPtr) {
    srandom(1000);
    int numSuggestions = 10;
    int suggestions[numSuggestions];
    float scores[numSuggestions];
    float totalSatisfaction = 0.0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        int userId = random()%datasetPtr->numUsers;
        while (userId == 53) userId = random()%datasetPtr->numUsers;
        User* userPtr = getUserById(datasetPtr, userId);
        computeBestMoviesForUser(numSuggestions, suggestions, scores, datasetPtr, userPtr->id, computeJaccardIndex);
        float arrayMoviesRatingsCopy[userPtr->numRatings];
        for (int j = 0; j < userPtr->numRatings; j++)
            arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
        quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
        float numerator = 0.0;
        float denominator = 0.0;
        for (int j = 0; j < numSuggestions; j++) {
            numerator += scores[j];
            denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-j];
        }
        float satisfaction = numerator/denominator;
        totalSatisfaction += satisfaction;
    }
    float averageSatisfaction = totalSatisfaction/50;
    printf("\naverage satisfaction (jaccard index): [%f]\n", averageSatisfaction);
}

void test7(Dataset* datasetPtr) {
    srandom(1000);
    int numSuggestions = 10;
    int suggestions[numSuggestions];
    float scores[numSuggestions];
    float totalSatisfaction = 0.0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        int userId = random()%datasetPtr->numUsers;
        while (userId == 53) userId = random()%datasetPtr->numUsers;
        User* userPtr = getUserById(datasetPtr, userId);
        computeBestMoviesForUser(numSuggestions, suggestions, scores, datasetPtr, userPtr->id, multiplyJaccardPearson);
        float arrayMoviesRatingsCopy[userPtr->numRatings];
        for (int j = 0; j < userPtr->numRatings; j++)
            arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
        quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
        float numerator = 0.0;
        float denominator = 0.0;
        for (int j = 0; j < numSuggestions; j++) {
            numerator += scores[j];
            denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-j];
        }
        float satisfaction = numerator/denominator;
        totalSatisfaction += satisfaction;
    }
    float averageSatisfaction = totalSatisfaction/50;
    printf("\naverage satisfaction (jaccard*pearson): [%f]\n", averageSatisfaction);
}

void test8(Dataset* datasetPtr) {
    srandom(1000);
    int numSuggestions = 10;
    int suggestions[numSuggestions];
    float scores[numSuggestions];
    float totalSatisfaction = 0.0;
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        int userId = random()%datasetPtr->numUsers;
        while (userId == 53) userId = random()%datasetPtr->numUsers;
        User* userPtr = getUserById(datasetPtr, userId);
        computeBestMoviesForUser(numSuggestions, suggestions, scores, datasetPtr, userPtr->id, multiplyJaccardPearson2);
        float arrayMoviesRatingsCopy[userPtr->numRatings];
        for (int j = 0; j < userPtr->numRatings; j++)
            arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
        quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
        float numerator = 0.0;
        float denominator = 0.0;
        for (int j = 0; j < numSuggestions; j++) {
            numerator += scores[j];
            denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-j];
        }
        float satisfaction = numerator/denominator;
        totalSatisfaction += satisfaction;
    }
    float averageSatisfaction = totalSatisfaction/50;
    printf("\naverage satisfaction (pearson*pow(jaccard)): [%f]\n", averageSatisfaction);
}

void test9(Dataset* datasetPtr) {
    srandom(1000);
    int numSuggestions = 10;
    int suggestions[numSuggestions];
    float scores[numSuggestions];
    float totalSatisfaction = 0.0;
    int groupSize = 3;
    int groupIds[groupSize];
    for (int i = 0; i < 50; i++) {
        printf("\r%d", i);
        fflush(stdout);
        int addedUsers = 0;
        while (addedUsers < groupSize) {
            int userId = random()%datasetPtr->numUsers;
            if (!isInIntArray(groupIds, userId, addedUsers) && userId != 53) {
                groupIds[addedUsers] = userId;
                addedUsers++;
            }
        }
        computeGroupRecommendationsAverage(numSuggestions, groupSize, groupIds, datasetPtr, computePearsonCorrelation, suggestions, scores);
        float groupAverageSatisfaction = 0.0;
        for (int j = 0; j < groupSize; j++) {
            User* userPtr = getUserById(datasetPtr, groupIds[j]);
            float arrayMoviesRatingsCopy[userPtr->numRatings];
            for (int j = 0; j < userPtr->numRatings; j++)
                arrayMoviesRatingsCopy[j] = userPtr->arrayMoviesRatings[j];
            quickSort(arrayMoviesRatingsCopy, userPtr->numRatings);
            float numerator = 0.0;
            float denominator = 0.0;
            for (int j = 0; j < numSuggestions; j++) {
                int movieId = suggestions[j];
                numerator += predictUserRatingForMovie(datasetPtr, userPtr->id, movieId, computePearsonCorrelation);
                denominator += arrayMoviesRatingsCopy[userPtr->numRatings-1-j];
            }
            float satisfaction = numerator/denominator;
            groupAverageSatisfaction += satisfaction;
        }
        groupAverageSatisfaction = groupAverageSatisfaction/groupSize;
        totalSatisfaction += groupAverageSatisfaction;
    }
    float averageSatisfaction = totalSatisfaction/50;
    printf("\naverage group satisfaction (average method): [%f]\n", averageSatisfaction);
}

int main() {
    // reading csv
    Dataset* datasetPtr = readCsv("ratings.csv");
    printf("number of users: [%d]\n", datasetPtr->numUsers);
    // test1(datasetPtr);
    // test2(datasetPtr);
    // test3(datasetPtr);
    // test4(datasetPtr);
    
    // test5(datasetPtr);
    test6(datasetPtr);
    test7(datasetPtr);
    test8(datasetPtr);
    
    test9(datasetPtr);
    
    freeDataset(datasetPtr);
    return 0;
}