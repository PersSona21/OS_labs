#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_SIZE 4096

char SHM_NAME[] = "/shared-memory";
char SEM_NAME[] = "/semaphore";

int main(int argc, char **argv) {
    // Открываем shared memory
    int shm = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm == -1) {
        const char msg[] = "ERROR: Failed to open SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Отображаем shared memory в виртуальную память
    char *shm_buf =
        mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "ERROR: Failed to map SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Открываем семафор
    sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    if (sem == SEM_FAILED) {
        const char msg[] = "ERROR: Failed to open semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // ждём данные от родителя
    bool running = true;
    while (running) {
        sem_wait(sem);
        uint32_t *length = (uint32_t *)shm_buf;
        char *data = shm_buf + sizeof(uint32_t);
        // код для выхода
        if (*length == UINT32_MAX) {
            running = false;
        }
        // это запрос от родителя
        else if (*length > 0 && *length < SHM_SIZE - sizeof(uint32_t)) {
            // Разбираем ввод исуммируем
            float sum = 0.0;
            int count = 0;
            char *p = data;
            char *endptr;
            while (*p && p < data + *length) {
                if ((*p == ' ' || *p == '\t') && *p != '\n') {
                    p++;
                    continue;
                }
                if (*p == '\n') {
                    break;
                }
                float num = strtof(p, &endptr);
                // больше чем float
                if (num == HUGE_VALF || num == -HUGE_VALF) {
                    const char msg[] = "ERROR: Number out of range\n";
                    memcpy(data, msg, sizeof(msg) - 1);
                    // Сообщаем родителю об ошибке: устанавливаем
                    // длину = длина_сообщения + SHM_SIZE (>= SHM_SIZE)
                    *length = (sizeof(msg) - 1) + SHM_SIZE;
                    sem_post(sem);
                    exit(EXIT_FAILURE);
                }
                // Не получилось ничего прочитать
                if (p == endptr) {
                    const char msg[] = "ERROR: Invalid character in input\n";
                    memcpy(data, msg, sizeof(msg) - 1);
                    *length = (sizeof(msg) - 1) + SHM_SIZE;
                    sem_post(sem);
                    exit(EXIT_FAILURE);
                }
                sum += num;
                count++;
                p = endptr;
            }
            // ничего не считали
            if (count == 0) {
                const char msg[] = "ERROR: No numbers provided\n";
                memcpy(data, msg, sizeof(msg) - 1);
                *length = (sizeof(msg) - 1) + SHM_SIZE;
                sem_post(sem);
                exit(EXIT_FAILURE);
            }

            // Открываем файл для записи
            int32_t file =
                open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0600);
            if (file == -1) {
                const char msg[] = "ERROR: Failed to open requested file\n";
                memcpy(data, msg, sizeof(msg) - 1);
                *length = (sizeof(msg) - 1) + SHM_SIZE;
                sem_post(sem);
                exit(EXIT_FAILURE);
            }

            // Форматируем сумму как строку
            char sum_str[64];
            int len = snprintf(sum_str, sizeof(sum_str), "%.2f\n", sum);

            // Записываем сумму в файл
            int32_t written = write(file, sum_str, len);
            if (written != len) {
                const char msg[] = "ERROR: Failed to write to file\n";
                memcpy(data, msg, sizeof(msg) - 1);
                *length = (sizeof(msg) - 1) + SHM_SIZE;
                sem_post(sem);
                exit(EXIT_FAILURE);
            }
            close(file);

            // Отправляем результат родителю через shared memory
            *length = len + SHM_SIZE;
            memcpy(data, sum_str, len);
        }
        sem_post(sem);
    }

    sem_close(sem);
    munmap(shm_buf, SHM_SIZE);
    close(shm);
    return 0;
}