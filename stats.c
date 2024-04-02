#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "datasetReader.h"
#include "project.h"
#include "utils.h"

int main() {
    srandom(1000);
    // reading csv
    Dataset* datasetPtr = readCsv("ratings.csv");
    printf("number of users: [%d]\n", datasetPtr->numUsers);
    float meanSquaredError = 0.0;
    int n = 0;
    for (int i = 0; i < 50; i++) {
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
    printf("mean squared error: [%f]\n", meanSquaredError);
    
    freeDataset(datasetPtr);
    return 0;
}