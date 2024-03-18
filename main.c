#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "csv.h"
#include "utils.h"

// assumes user id is in csv file and csv file is ordered by id
int find_user_row_index(Csv* csv_ptr, int user_id) {
    int start = 1;
    int end = csv_ptr->size;
    int row_index = (start+end)/2;
    while (1) {
        if (start+1 == end) break;
        char* string_id = get_row_field(csv_ptr, 0, row_index);
        int id = atoi(string_id);
        free(string_id);
        if (id<user_id) {
            start = row_index;
            row_index = (start+end)/2;
            continue;
        }
        if (id>user_id) {
            end = row_index;
            row_index = (start+end)/2;
            continue;
        }
        while (row_index>0) {
            string_id = get_row_field(csv_ptr, 0, row_index-1);
            id = atoi(string_id);
            free(string_id);
            if (id != user_id) break;
            row_index--;
        }
        break;
    }
    return row_index;
}

void get_movies_ids_ratings_by_user(int user_id, Csv* csv_ptr, int starting_index, int** array_user_movies_ids_ptr,
                                    int** array_user_movies_ratings_ptr, int* array_len_ptr) {
    *array_len_ptr = 0;
    while (1) {
        if (starting_index+(*array_len_ptr) == csv_ptr->size) break;
        char* string_id = get_row_field(csv_ptr, 0, starting_index+(*array_len_ptr));
        int id = atoi(string_id);
        free(string_id);
        if (id != user_id)
            break;
        *array_len_ptr = (*array_len_ptr)+1;
    }
    *array_user_movies_ids_ptr = (int*)malloc((*array_len_ptr)*sizeof(int));
    *array_user_movies_ratings_ptr = (int*)malloc((*array_len_ptr)*sizeof(int));
    for (int i = 0; i < *array_len_ptr; i++) {
        char* string_movie_id = get_row_field(csv_ptr, 1, starting_index+i);
        int movie_id = atoi(string_movie_id);
        free(string_movie_id);
        (*array_user_movies_ids_ptr)[i] = movie_id;
        char* string_movie_rating = get_row_field(csv_ptr, 2, starting_index+i);
        int movie_rating = atoi(string_movie_rating);
        free(string_movie_rating);
        (*array_user_movies_ratings_ptr)[i] = movie_rating;
    }
}

float compute_pearson_correlation(int user_1_array_len, int* array_user_1_movies_ratings, int* array_user_1_movies_ids,
                                  int user_2_array_len, int* array_user_2_movies_ratings, int* array_user_2_movies_ids) {
    int sum_user_1_ratings = 0; // of movies in common
    int sum_user_2_ratings = 0; // of movies in common
    float n_movies_in_common = 0;
    for (int j = 0; j < user_1_array_len; j++) {
        int pos = pos_in_int_array(array_user_2_movies_ids, user_2_array_len, array_user_1_movies_ids[j]);
        if (pos == -1) continue;
        sum_user_1_ratings += array_user_1_movies_ratings[j];
        sum_user_2_ratings += array_user_2_movies_ratings[pos];
        n_movies_in_common++;
    }
    float avg_user_1_ratings = sum_user_1_ratings / n_movies_in_common;
    float avg_user_2_ratings = sum_user_2_ratings / n_movies_in_common;
    float numerator = 0;
    float denominator_1 = 0;
    float denominator_2 = 0;
    for (int j = 0; j < user_1_array_len; j++) {
        int pos = pos_in_int_array(array_user_2_movies_ids, user_2_array_len, array_user_1_movies_ids[j]);
        if (pos == -1) continue;
        float x = (float)array_user_1_movies_ratings[j];
        float y = (float)array_user_2_movies_ratings[pos];
        numerator += (x-avg_user_1_ratings)*(y-avg_user_2_ratings);
        denominator_1 += (x-avg_user_1_ratings)*(x-avg_user_1_ratings);
        denominator_2 += (y-avg_user_2_ratings)*(y-avg_user_2_ratings);
    }
    return numerator / sqrt(denominator_1*denominator_2);
}

float compute_jaccard_index(int user_1_array_len, int* array_user_1_movies_ratings, int* array_user_1_movies_ids,
                            int user_2_array_len, int* array_user_2_movies_ratings, int* array_user_2_movies_ids) {
    int numerator = 0.0;
    int denominator = 0.0;
    for (int i = 0; i < user_1_array_len; i++) {
        denominator++;
        if (is_in_int_array(array_user_2_movies_ids, array_user_1_movies_ids[i], user_2_array_len))
            numerator++;
    }
    for (int i = 0; i < user_2_array_len; i++)
        if (!is_in_int_array(array_user_1_movies_ids, array_user_2_movies_ids[i], user_2_array_len))
            denominator++;
    return numerator/(float)denominator;
}

void compute_top_n_similar_users(int n, Csv* csv_ptr, int user_id, int* array_top_n_users_ids, 
                                 float* array_top_n_user_scores, int* number_of_top_users_ptr,
                                 float (*function_compute_similarity)(int, int*, int*, int, int*, int*)) {
    *number_of_top_users_ptr = 0;
    int* array_user_movies_ids;
    int* array_user_movies_ratings;
    int array_len;
    int user_row_index = find_user_row_index(csv_ptr, user_id);
    get_movies_ids_ratings_by_user(user_id, csv_ptr, user_row_index, &array_user_movies_ids, &array_user_movies_ratings, &array_len);
    for (int row_index = 1; row_index < csv_ptr->size; row_index++) { // starting index = 1 since index 0 is for labels
        char* string_new_user_id = get_row_field(csv_ptr, 0, row_index);
        int new_user_id = atoi(string_new_user_id);
        free(string_new_user_id);
        if (new_user_id == user_id) continue;
        int* array_new_user_movies_ids;
        int* array_new_user_movies_ratings;
        int new_user_array_len;
        get_movies_ids_ratings_by_user(new_user_id, csv_ptr, row_index, &array_new_user_movies_ids, 
                                       &array_new_user_movies_ratings, &new_user_array_len);
        float score = function_compute_similarity(array_len, array_user_movies_ratings, array_user_movies_ids,
                                         new_user_array_len, array_new_user_movies_ratings, array_new_user_movies_ids);
        int pos = -1;
        if (!isnan(score)) {
            pos = insert_in_float_sorted_array(array_top_n_user_scores, *number_of_top_users_ptr-1, score, n);
            if (pos != -1) {
                insert_in_int_array(array_top_n_users_ids, n, pos, new_user_id);
                if (*number_of_top_users_ptr != n) *number_of_top_users_ptr = *number_of_top_users_ptr + 1;
            }
        }
        free(array_new_user_movies_ids);
        free(array_new_user_movies_ratings);
        if (pos == n-1 && array_top_n_user_scores[pos] == 1.0) break;
        row_index += new_user_array_len-1;
    }
    free(array_user_movies_ids);
    free(array_user_movies_ratings);
}

float compute_similarity_between_users_by_id(Csv* csv_ptr, int user_1_id, int user_2_id, 
                                             float (*function_compute_similarity)(int, int*, int*, int, int*, int*)) {
    int row_index_user_1 = find_user_row_index(csv_ptr, user_1_id);
    int* array_user_1_movies_ids;
    int* array_user_1_movies_ratings;
    int user_1_array_len;
    get_movies_ids_ratings_by_user(user_1_id, csv_ptr, row_index_user_1, &array_user_1_movies_ids, &array_user_1_movies_ratings, &user_1_array_len);
    int row_index_user_2 = find_user_row_index(csv_ptr, user_2_id);
    int* array_user_2_movies_ids;
    int* array_user_2_movies_ratings;
    int user_2_array_len;
    get_movies_ids_ratings_by_user(user_2_id, csv_ptr, row_index_user_2, &array_user_2_movies_ids, &array_user_2_movies_ratings, &user_2_array_len);
    float similarity = function_compute_similarity(user_1_array_len, array_user_1_movies_ratings, array_user_1_movies_ids,
                                       user_2_array_len, array_user_2_movies_ratings, array_user_2_movies_ids);
    free(array_user_1_movies_ids);
    free(array_user_1_movies_ratings);
    free(array_user_2_movies_ids);
    free(array_user_2_movies_ratings);
    return similarity;
}

int get_user_movie_rating(Csv* csv_ptr, int user_id, int movie_id, int user_row_index) {
    for (int row_index = user_row_index; row_index < csv_ptr->size; row_index++) {
        char* string_new_user_id = get_row_field(csv_ptr, 0, row_index);
        int new_user_id = atoi(string_new_user_id);
        free(string_new_user_id);
        if (new_user_id != user_id) return -1;
        char* string_current_movie_id = get_row_field(csv_ptr, 1, row_index);
        int current_movie_id = atoi(string_current_movie_id);
        free(string_current_movie_id);
        if (current_movie_id == movie_id) {
            char* string_movie_rating = get_row_field(csv_ptr, 2, row_index);
            int movie_rating = atoi(string_movie_rating);
            free(string_movie_rating);
            return movie_rating;
        }
    }
    return -1;
}

float predict_user_rating_for_movie(Csv* csv_ptr, int user_id, int movie_id, int top_n_user_number,
                                    float (*function_compute_similarity)(int, int*, int*, int, int*, int*)) {
    int* array_user_movies_ids;
    int* array_user_movies_ratings;
    int array_len;
    int user_row_index = find_user_row_index(csv_ptr, user_id);
    get_movies_ids_ratings_by_user(user_id, csv_ptr, user_row_index, &array_user_movies_ids, &array_user_movies_ratings, &array_len);
    float avg_rating_user = 0.0;
    for (int i = 0; i < array_len; i++)
        avg_rating_user += array_user_movies_ratings[i];
    avg_rating_user = avg_rating_user / array_len;
    float array_top_n_users_scores[top_n_user_number];
    int array_top_n_users_ids[top_n_user_number];
    float array_top_n_user_avg_ratings[top_n_user_number];
    int last_elem_index = -1;
    for (int row_index = 1; row_index < csv_ptr->size; row_index++) { // starting index = 1 since index 0 is for labels
        char* string_new_user_id = get_row_field(csv_ptr, 0, row_index);
        int new_user_id = atoi(string_new_user_id);
        free(string_new_user_id);
        if (new_user_id == user_id) continue;
        int has_new_user_rated_movie = (get_user_movie_rating(csv_ptr, new_user_id, movie_id, row_index) >= 0);
        if (!has_new_user_rated_movie) {
            while (row_index+1 < csv_ptr->size) {
                string_new_user_id = get_row_field(csv_ptr, 0, row_index+1);
                int temp_user_id = atoi(string_new_user_id);
                free(string_new_user_id);
                if (temp_user_id == new_user_id) row_index++;
                else break;
            }
            continue;
        }
        int* array_new_user_movies_ids;
        int* array_new_user_movies_ratings;
        int new_user_array_len;
        get_movies_ids_ratings_by_user(new_user_id, csv_ptr, row_index, &array_new_user_movies_ids, 
                                       &array_new_user_movies_ratings, &new_user_array_len);
        float score = function_compute_similarity(array_len, array_user_movies_ratings, array_user_movies_ids,
                                        new_user_array_len, array_new_user_movies_ratings, array_new_user_movies_ids);
        int pos = -1;
        if (!isnan(score)) {
            pos = insert_in_float_sorted_array(array_top_n_users_scores, last_elem_index, score, top_n_user_number);
            if (pos != -1) {
                insert_in_int_array(array_top_n_users_ids, top_n_user_number, pos, new_user_id);
                if (last_elem_index < top_n_user_number-1) last_elem_index++;
                float avg_rating_similar_user = 0.0;
                for (int i = 0; i < new_user_array_len; i++)
                    avg_rating_similar_user += array_new_user_movies_ratings[i];
                avg_rating_similar_user = avg_rating_similar_user / new_user_array_len;
                insert_in_float_array(array_top_n_user_avg_ratings, top_n_user_number, pos, avg_rating_similar_user);
            }
        }
        free(array_new_user_movies_ids);
        free(array_new_user_movies_ratings);
        if (pos == top_n_user_number-1 && array_top_n_users_scores[pos] == 1.0) break;
        row_index += new_user_array_len-1;
    }
    float predict_numerator = 0.0;
    float predict_denominator = 0.0;
    for (int i = 0; i <= last_elem_index; i++) {
        int similar_user_id = array_top_n_users_ids[i];
        float similar_user_score = array_top_n_users_scores[i];
        float avg_similar_user_score = array_top_n_user_avg_ratings[i];
        if (similar_user_score < 0.3) break;
        int row_index = find_user_row_index(csv_ptr, similar_user_id);
        int rating = get_user_movie_rating(csv_ptr, similar_user_id, movie_id, row_index);
        predict_numerator += (similar_user_score*(rating-avg_similar_user_score));
        predict_denominator += similar_user_score;
    }
    free(array_user_movies_ids);
    free(array_user_movies_ratings);
    if (predict_denominator == 0.0) return avg_rating_user;
    return avg_rating_user + (predict_numerator/predict_denominator);
}

void compute_best_n_movies_for_user(int n, int* array_best_n_movies_ids, float* array_best_n_movies_scores, int* num_best_movies_ptr,
                                    Csv* csv_ptr, int user_id, float (*function_compute_similarity)(int, int*, int*, int, int*, int*)) {
    int array_top_similar_users_ids[n];
    float array_top_similar_users_scores[n];
    int n_top_similar_users;
    compute_top_n_similar_users(n, csv_ptr, user_id, array_top_similar_users_ids, array_top_similar_users_scores,
                                &n_top_similar_users, function_compute_similarity);
    int user_row_index = find_user_row_index(csv_ptr, user_id);
    *num_best_movies_ptr = 0;
    for (int i = 0; i < n_top_similar_users; i++) {
        int similar_user_id = array_top_similar_users_ids[i];
        int row_index = find_user_row_index(csv_ptr, similar_user_id);
        for (int j = row_index; j < csv_ptr->size; j++) {
            char* string_similar_user_id = get_row_field(csv_ptr, 0, j);
            int temp_similar_user_id = atoi(string_similar_user_id);
            free(string_similar_user_id);
            if (similar_user_id != temp_similar_user_id) break;
            char* string_movie_id = get_row_field(csv_ptr, 1, j);
            int movie_id = atoi(string_movie_id);
            free(string_movie_id);
            if (is_in_int_array(array_best_n_movies_ids, movie_id, *num_best_movies_ptr))
                continue;
            if (get_user_movie_rating(csv_ptr, user_id, movie_id, user_row_index) == -1) {
                float score = predict_user_rating_for_movie(csv_ptr, user_id, movie_id, n, function_compute_similarity);
                int pos = insert_in_float_sorted_array(array_best_n_movies_scores, (*num_best_movies_ptr)-1, score, n);
                if (pos != -1) {
                    insert_in_int_array(array_best_n_movies_ids, *num_best_movies_ptr, pos, movie_id);
                    if (*num_best_movies_ptr < n) *num_best_movies_ptr = *num_best_movies_ptr+1;
                    float avg_rating_similar_user = 0.0;
                }
            }
        }
    }
}

void compute_group_recommendations(int group_size, int* array_group_ids, Csv* csv_ptr, float (*function_compute_similarity)(int, int*, int*, int, int*, int*),
                                    float (*aggregation_function)(float*, int), int* array_movies_ids_output, float* array_movies_scores_output) {
    int* array_of_arrays_movies_suggestions_for_user[group_size];
    int num_best_movies_for_user[group_size];
    for (int i = 0; i < group_size; i++) {
        int* array_best_movies_ids = (int*)malloc(5*sizeof(int));
        float array_best_movies_scores[5];
        int num_best_movies;
        compute_best_n_movies_for_user(5, array_best_movies_ids, array_best_movies_scores, &num_best_movies, csv_ptr, array_group_ids[i], function_compute_similarity);
        array_of_arrays_movies_suggestions_for_user[i] = array_best_movies_ids;
        num_best_movies_for_user[i] = num_best_movies;
    }
    int array_movies_ids[5*group_size];
    float* array_of_arrays_movies_scores_for_all_users[5*group_size];
    int i = 0;
    for (int j = 0; j < group_size; j++) {
        int* array_best_movies_ids = array_of_arrays_movies_suggestions_for_user[j];
        for (int k = 0; k < num_best_movies_for_user[j]; k++) {
            int movie_id = array_best_movies_ids[k];
            if (is_in_int_array(array_movies_ids, movie_id, i)) continue;
            array_movies_ids[i] = movie_id;
            float* array_movies_scores = (float*)malloc(sizeof(float)*group_size);
            for (int l = 0; l < group_size; l++) {
                int user_id = array_group_ids[l];
                int row = find_user_row_index(csv_ptr, user_id);
                int rating = get_user_movie_rating(csv_ptr, user_id, movie_id, row);
                if (rating == -1) {
                    float predict = predict_user_rating_for_movie(csv_ptr, user_id, movie_id, 5, function_compute_similarity);
                    array_movies_scores[l] = predict;
                }
                else
                    array_movies_scores[l] = (float)rating;
            }
            array_of_arrays_movies_scores_for_all_users[i] = array_movies_scores;
            i++;
        }
        free(array_best_movies_ids);
    }
    float array_movies_aggregated_scores[5*group_size];
    for (int j = 0; j < i; j++) {
        float aggregated_score = aggregation_function(array_of_arrays_movies_scores_for_all_users[j], group_size);
        free(array_of_arrays_movies_scores_for_all_users[j]);
        array_movies_aggregated_scores[j] = aggregated_score;
    }
    for (int j = 0; j < 10; j++) {
        int pos = find_max_position_in_float_array(array_movies_aggregated_scores, i);
        array_movies_ids_output[j] = array_movies_ids[pos];
        array_movies_scores_output[j] = array_movies_aggregated_scores[pos];
        array_movies_aggregated_scores[pos] = 0.0;
    }
}

#define N 10

int main() {
    time_t start, end;
    double elapsed;
    start = clock();

    Csv* csv_ptr = read_csv("ratings.csv");
    printf("csv number of rows: [%d]\n", csv_ptr->size);
    int user_id = 500;
    int array_top_similar_users_ids[N];
    float array_top_similar_users_scores[N];
    int n=0;
    compute_top_n_similar_users(N, csv_ptr, user_id, array_top_similar_users_ids, array_top_similar_users_scores, &n, compute_pearson_correlation);
    printf("\nmost similar users:\n");
    print_int_array(array_top_similar_users_ids, n);
    printf("scores:\n");
    print_float_array(array_top_similar_users_scores, n);

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
    compute_best_n_movies_for_user(N, array_best_n_movies_ids, array_best_n_movies_scores, &num_best_movies, csv_ptr, user_id, compute_pearson_correlation);
    printf("\nbest movies ids:\n");
    print_int_array(array_best_n_movies_ids, num_best_movies);
    printf("best movies predicted scores:\n");
    print_float_array(array_best_n_movies_scores, num_best_movies);
    if (num_best_movies > 0) {
        predict = predict_user_rating_for_movie(csv_ptr, user_id, array_best_n_movies_ids[0], N, compute_pearson_correlation);
        printf("\npredict user [%d] movie [%d] rating: [%f]\n", user_id, array_best_n_movies_ids[0], predict);
    }
    
    int group_size = 3;
    int array_group_ids[3] = {1, 2, 500};
    int array_movies_ids[10];
    float array_aggregated_scores[10];
    compute_group_recommendations(group_size, array_group_ids, csv_ptr, compute_pearson_correlation, average_of_float_array, array_movies_ids, array_aggregated_scores);  
    printf("\ngroup recommendations (pearson coefficient + average aggregation):\nbest movies ids:\n");
    print_int_array(array_movies_ids, 10);
    printf("best movies predicted scores:\n");
    print_float_array(array_aggregated_scores, 10);

    compute_group_recommendations(group_size, array_group_ids, csv_ptr, compute_jaccard_index, find_min_in_float_array, array_movies_ids, array_aggregated_scores);  
    printf("\ngroup recommendations (jaccard index + least misery):\nbest movies ids:\n");
    print_int_array(array_movies_ids, 10);
    printf("best movies predicted scores:\n");
    print_float_array(array_aggregated_scores, 10);

    free_csv(csv_ptr);

    end = clock();
    elapsed = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("\ntotal cpu time: [%f sec]\n", elapsed);
    return 0;
}