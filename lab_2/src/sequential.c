#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <filename> <N> <K>\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    if (N < 3 || K < 1) {
        fprintf(stderr, "N >= 3 and K >= 1 required\n");
        return 1;
    }

    // Выделяем две матрицы (ping-pong)
    float **A = malloc(N * sizeof(float *));
    float **B = malloc(N * sizeof(float *));
    for (int i = 0; i < N; i++) {
        A[i] = malloc(N * sizeof(float));
        B[i] = malloc(N * sizeof(float));
    }

    // Заполняем случайными значениями [0, 1]
    srand(time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = (float)rand() / RAND_MAX;

    // Замеряем время
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Если K нечетное то out, иначе in
    float **in = A;
    float **out = B;

    for (int k = 0; k < K; k++) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {

                float sum = 0.0;
                int count = 0;

                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {

                        int ni = i + di;
                        int nj = j + dj;

                        // Считаем сумму
                        if (ni >= 0 && ni < N && nj >= 0 && nj < N) {
                            sum += in[ni][nj];
                            count++;
                        }
                    }
                }
                out[i][j] = sum / count; // Считаем среднее
            }
        }
        // Меняем указатели
        float **tmp = in;
        in = out;
        out = tmp;
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // Конец подсчета времени
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_nsec - start.tv_nsec) / 1e6;

    printf("=== Sequential version ===\n");
    printf("N = %d, K = %d\n", N, K);
    printf("Time: %.2f ms\n", time_ms);

    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    return 0;
}