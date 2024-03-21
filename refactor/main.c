#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "csv.h"
#include "utils.h"


float compute_jaccard_index(int user_1_array_len, int* array_user_1_movies_ratings, int* array_user_1_movies_ids,
                            int user_2_array_len, int* array_user_2_movies_ratings, int* array_user_2_movies_ids) {
    int numerator = 0;
    int denominator = user_1_array_len;
    for (int i = 0; i < user_1_array_len; i++)
        if (isInIntArray(array_user_2_movies_ids, array_user_1_movies_ids[i], user_2_array_len))
            numerator++;
    for (int i = 0; i < user_2_array_len; i++)
        if (!isInIntArray(array_user_1_movies_ids, array_user_2_movies_ids[i], user_1_array_len))
            denominator++;
    return 2*(0.5-numerator/(float)denominator);
}


void compute_group_recommendations_with_disagreement(int group_size, int* array_group_ids, Dataset* csv_ptr, int* array_movies_ids_output,
                                    float (*function_compute_similarity)(int, int*, int*, int, int*, int*), float* array_movies_scores_output,
                                    float (*aggregation_function)(float*, int)) {
    int array_movies_ids[10];
    float array_aggregated_scores[10];
    compute_group_recommendations(group_size, array_group_ids, csv_ptr, function_compute_similarity, aggregation_function, array_movies_ids, array_aggregated_scores);
    int* array_of_arrays_user_movies_order_preference[group_size];
    for (int i = 0; i < group_size; i++) {
        int user_id = array_group_ids[i];
        int* array_user_movies_order_preference = (int*)malloc(sizeof(int)*10);
        float array_user_movies_scores[10];
        for (int j = 0; j < 10; j++) {
            int movie_id = array_movies_ids[j];
            float predict = predict_user_rating_for_movie(csv_ptr, user_id, movie_id, 10, function_compute_similarity);
            int pos = insert_in_float_sorted_array(array_user_movies_scores, j-1, predict, 10);
            insertInIntArray(array_user_movies_order_preference, 10, pos, movie_id);
        }
        array_of_arrays_user_movies_order_preference[i] = array_user_movies_order_preference;
    }
    int array_movies_ids_copy[10];
    for (int i = 0; i < 10; i++)
        array_movies_ids_copy[i] = array_movies_ids[i];
    for (int i = 0; i < 10; i++) {
        int j = 0;
        while (array_movies_ids_copy[j] == -1) j++;
        int candidate_movie_id = array_movies_ids_copy[j];
        int candidate_movie_id_total_disagreement = 0;
        for (int k = 0; k < group_size; k++) {
            int pos = pos_in_int_array(array_of_arrays_user_movies_order_preference[k], 10, candidate_movie_id);
            candidate_movie_id_total_disagreement += abs(i-pos);
        }
        for (int k = j+1; k < 10; k++) {
            int movie_id = array_movies_ids_copy[k];
            if (movie_id == -1) continue;
            int disagreement = 0;
            for (int l = 0; l < group_size; l++) {
                int pos = pos_in_int_array(array_of_arrays_user_movies_order_preference[l], 10, candidate_movie_id);
                disagreement += abs(i-pos);
            }
            if (disagreement < candidate_movie_id_total_disagreement) {
                candidate_movie_id_total_disagreement = disagreement;
                candidate_movie_id = movie_id;
            }
        }
        array_movies_ids_output[i] = candidate_movie_id;
        int pos = pos_in_int_array(array_movies_ids_copy, 10, candidate_movie_id);
        array_movies_ids_copy[pos] = -1;
    }
    for (int i = 0; i < 10; i++) {
        int movie_id = array_movies_ids_output[i];
        int pos = pos_in_int_array(array_movies_ids, 10, movie_id);
        array_movies_scores_output[i] = array_aggregated_scores[pos];
    }
}

#define N 10

int main() {
    time_t start, end;
    double elapsed;
    start = clock();

    Dataset* csv_ptr = read_csv("ratings.csv");
    printf("csv number of rows: [%d]\n", csv_ptr->size);
    int user_id = 500;
    int array_top_similar_users_ids[N];
    float array_top_similar_users_scores[N];
    int n=0;
    compute_top_n_similar_users(N, csv_ptr, user_id, array_top_similar_users_ids, array_top_similar_users_scores, &n, compute_pearson_correlation);
    printf("\nmost similar users to user [%d]:\n", user_id);
    printIntArray(array_top_similar_users_ids, n);
    printf("scores:\n");
    printFloatArray(array_top_similar_users_scores, n);

    int user_1_id = 43;
    int user_2_id = 500;
    float similarity =  compute_similarity_between_users_by_id(csv_ptr, user_1_id, user_2_id, compute_pearson_correlation);
    printf("\nsimilarity between user [%d] and user [%d]: [%f]\n", user_1_id, user_2_id, similarity);

    int movie_id = 471;
    float predict = predict_user_rating_for_movie(csv_ptr, user_id, movie_id, N, compute_pearson_correlation);
    printf("predict user [%d] movie [%d] rating: [%f]\n", user_id, movie_id, predict);

    int array_best_n_movies_ids[N];
    float array_best_n_movies_scores[N];
    int num_best_movies;
    printf("\nmovies suggestions for user [%d]\n", user_id);
    compute_best_n_movies_for_user(N, array_best_n_movies_ids, array_best_n_movies_scores, &num_best_movies, csv_ptr, user_id, compute_pearson_correlation);
    printf("best movies ids:\n");
    printIntArray(array_best_n_movies_ids, num_best_movies);
    printf("best movies predicted scores:\n");
    printFloatArray(array_best_n_movies_scores, num_best_movies);
    
    int group_size = 3;
    int array_group_ids[3] = {10, 37, 500};
    int array_movies_ids[10];
    float array_aggregated_scores[10];
    printf("\ngroup: ");
    printIntArray(array_group_ids, group_size);
    compute_group_recommendations(group_size, array_group_ids, csv_ptr, compute_pearson_correlation, average_of_float_array, array_movies_ids, array_aggregated_scores);  
    printf("group recommendations (pearson coefficient + average aggregation):\nbest movies ids:\n");
    printIntArray(array_movies_ids, 10);
    printf("best movies predicted scores:\n");
    printFloatArray(array_aggregated_scores, 10);

    compute_group_recommendations(group_size, array_group_ids, csv_ptr, compute_jaccard_index, find_min_in_float_array, array_movies_ids, array_aggregated_scores);  
    printf("\ngroup recommendations (jaccard index + least misery):\nbest movies ids:\n");
    printIntArray(array_movies_ids, 10);
    printf("best movies predicted scores:\n");
    printFloatArray(array_aggregated_scores, 10);

    compute_group_recommendations_with_disagreement(group_size, array_group_ids, csv_ptr, array_movies_ids, compute_pearson_correlation, array_aggregated_scores, average_of_float_array);  
    printf("\ngroup recommendations (pearson coefficient + average aggregation + disagreement):\nbest movies ids:\n");
    printIntArray(array_movies_ids, 10);
    printf("best movies predicted scores:\n");
    printFloatArray(array_aggregated_scores, 10);

    free_dataset(csv_ptr);

    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\ntotal cpu time: [%f sec]\n", elapsed);
    return 0;
}