#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <fcntl.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_SIZE 4096

char SHM_NAME[] = "/shared-memory";
char SEM_NAME[] = "/semaphore";

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
    if (argc != 2) {
        char msg[1024];
        uint32_t len =
            snprintf(msg, sizeof(msg) - 1, "usage: %s <filename>\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_SUCCESS);
    }

    // Получение пути
    char progpath[2048];
    uint32_t size = sizeof(progpath);
    if (_NSGetExecutablePath(progpath, &size) != 0) {
        const char msg[] = "ERROR: Failed to get executable path\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    ssize_t len = strlen(progpath);
    while (progpath[len] != '/') {
        --len;
    }
    progpath[len] = '\0';

    // Удаляем существующие shared memory и semaphore (если были)
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    // Создаём shared memory (вирт память), возвращаяет fd
    int shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (shm == -1) {
        const char msg[] = "ERROR: Failed to create SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Устанавливаем размер разделяемой памяти
    if (ftruncate(shm, SHM_SIZE) == -1) {
        const char msg[] = "ERROR: Failed to resize SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Отображаем shared memory в виртуальную память,
    // чтобы использоват его как массив
    char *shm_buf =
        mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "ERROR: Failed to map SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Создаём семафор (его операции атомарны)
    sem_t *sem = sem_open(SEM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
    if (sem == SEM_FAILED) {
        const char msg[] = "ERROR: Failed to create semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Создаём новый процесс
    const pid_t child = fork();

    if (child == -1) {
        // Не удалось создать новый процесс

        const char msg[] = "ERROR: Failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    } else if (child == 0) {
        // В дочернем процессе

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", progpath, CHILD_PROGRAM_NAME);

        char *const args[] = {CHILD_PROGRAM_NAME, argv[1], NULL};

        int32_t status = execv(path, args);

        if (status == -1) {
            const char msg[] =
                "ERROR: Failed to exec into new executable image\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
    }

    // В родительском процессе
    char buf[SHM_SIZE - sizeof(uint32_t)];
    ssize_t bytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes < 0) {
        const char msg[] = "ERROR: Failed to read from stdin\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // Отправляем данные дочернему процессу через shared memory
    sem_wait(sem);
    // Указатель на длину данных
    uint32_t *length = (uint32_t *)shm_buf;
    // Указатель на данные
    char *data = shm_buf + sizeof(uint32_t);
    if (bytes > 0) {
        // Пишем данные в виртуальную память
        *length = (uint32_t)bytes;
        memcpy(data, buf, bytes);
        sem_post(sem);

        // Ждём, пока дочерний процесс обработает данные и вернёт результат
        bool running = true;
        while (running) {
            // захватываем память
            sem_wait(sem);
            uint32_t len = *length;
            if (len >= SHM_SIZE) {
                ssize_t result_len = len - SHM_SIZE;
                *length = 0;
                // Данные прочитаны, отпускаем семафор
                sem_post(sem);
                // Проверяем, начинается ли результат с "ERROR:"
                const char error_prefix[] = "ERROR:";
                int output_fd = STDOUT_FILENO;
                if (result_len >= sizeof(error_prefix) - 1 &&
                    strncmp(data, error_prefix, sizeof(error_prefix) - 1) ==
                        0) {
                    output_fd = STDERR_FILENO;
                }
                // Выводим результат
                write(output_fd, data, result_len);
                running = false;
            } else {
                sem_post(sem);
            }
        }
        // Сигнализируем дочернему процессу о завершении
        sem_wait(sem);
        *length = UINT32_MAX;
        sem_post(sem);
    } else {
        // Нет входных данных: сигнализируем дочернему процессу о завершении
        *length = UINT32_MAX;
        sem_post(sem);
    }

    // Ждём завершения дочернего процесса
    int status;
    if (wait(&status) == -1) {
        const char msg[] = "ERROR: Failed to wait for child\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 0) {
            exit(EXIT_FAILURE);
        }
    } else {
        // Дочерний процесс завершился ненормально
        exit(EXIT_FAILURE);
    }

    sem_unlink(SEM_NAME);
    sem_close(sem);

    munmap(shm_buf, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm);
}