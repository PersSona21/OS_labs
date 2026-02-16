#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    float **in;
    float **out;
    int start_row;
    int end_row;
    int N;
    int K;
} ThreadData;

static void *thread_work(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    float **in = data->in;
    float **out = data->out;
    int N = data->N;

    for (int k = 0; k < data->K; k++) {
        for (int i = data->start_row; i < data->end_row; i++) {
            for (int j = 0; j < N; j++) {

                float sum = 0.0f;
                int count = 0;
                // Квадрат 3x3
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {

                        int ni = i + di;
                        int nj = j + dj;

                        if (ni >= 0 && ni < N && nj >= 0 && nj < N) {
                            sum += in[ni][nj];
                            count++;
                        }
                    }
                }
                out[i][j] = sum / count;
            }
        }
        // Меняем указатели
        float **tmp = in;
        in = out;
        out = tmp;
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: <filename> <N> <K> <max_threads>\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    int max_threads = atoi(argv[3]);

    if (N < 3 || K < 1 || max_threads < 1) {
        fprintf(stderr, "N >= 3, K >= 1, max_threads >= 1 required\n");
        return 1;
    }

    // Ограничиваем количество потоков
    int num_threads = max_threads;
    if (num_threads > N)
        num_threads = N; // не больше чем количество строк

    printf("=== Parallel version ===\n");
    printf("N = %d, K = %d, threads = %d\n", N, K, num_threads);

    // Выделяем матрицы
    float **A = malloc(N * sizeof(float *));
    float **B = malloc(N * sizeof(float *));
    for (int i = 0; i < N; i++) {
        A[i] = malloc(N * sizeof(float));
        B[i] = malloc(N * sizeof(float));
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = (float)rand() / RAND_MAX;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Создание потоков
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *tdata = malloc(num_threads * sizeof(ThreadData));

    float **in = A;
    float **out = B;

    int rows_per_thread = N / num_threads;
    int remainder = N % num_threads;

    for (int t = 0; t < num_threads; t++) {
        int start_row = t * rows_per_thread + (t < remainder ? t : remainder);
        int end_row = start_row + rows_per_thread + (t < remainder ? 1 : 0);

        tdata[t] = (ThreadData){.in = in,
                                .out = out,
                                .start_row = start_row,
                                .end_row = end_row,
                                .N = N,
                                .K = K};

        // Создание потока
        pthread_create(&threads[t], NULL, thread_work, &tdata[t]);
    }

    // Ожидание завершения потоков
    for (int t = 0; t < num_threads; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_nsec - start.tv_nsec) / 1e6;

    printf("Time: %.2f ms\n", time_ms);

    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);
    free(threads);
    free(tdata);

    return 0;
}