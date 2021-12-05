#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

int *N;
typedef struct arguments {
    int points;
    int i;
} Arg;

double get_rand() { // return random double from 0 to 1
    return ((double) rand()) / RAND_MAX;
}

double get_rand_range(double min, double max) { // return random double from min to max
    return get_rand() * (max - min) + min;
}

void *thread_function(void *args) { // create n random points and cheak
    Arg *arg = (Arg *) args;
    int n = arg->points;
    int i = arg->i;
    for (int j = 0; j < n; j++) {
        double x = get_rand_range(0, 52);
        double y = get_rand_range(0, 51);
        for (int k = 0; k < 52; k += 4) {
            if (k <= x && x < k + 4 && k <= y && y < k + 3) N[i]++;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Syntax: ./*executable_file_name* Rounds Number_of_threads\n");
        exit(1);
    }
    int Rounds = atoi(argv[1]), threads_num = atoi(argv[2]);
    N = (int *) calloc(threads_num, sizeof(int)); // array of number points
    srand(time(NULL));
    pthread_t *threads = (pthread_t *) calloc(threads_num, sizeof(pthread_t));
    if (threads == NULL) {
        printf("Can't allocate memory for threads\n");
        exit(1);
    }
    int points_for_thread = Rounds / threads_num;
    Arg a;
    for (int i = 0; i < threads_num; i++) {
        a.points = points_for_thread + (i < Rounds % threads_num);
        a.i = i;
        if (pthread_create(&threads[i], NULL, thread_function, &a) != 0) {
            printf("Can not create thread\n");
            exit(1);
        }
    }
    for (int i = 0; i < threads_num; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Join error\n");
            exit(1);
        }
    }
    double n = 0;
    for (int i = 0; i < threads_num; i++) { // calculate points
        n += (double) N[i] / Rounds;
    }
    printf("Monte-Carlo chance is %.5f\n", (double) n);
    printf("Real chance is %.5f\n", (double) 1 / 17);
    free(threads);
    return 0;
}