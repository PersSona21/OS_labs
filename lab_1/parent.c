#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char *argv[]) {
    if (argc == 1) {
        char msg[256];
        uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n",
                                argv[0]); // длина строки
        write(STDERR_FILENO, msg, len);
        exit(EXIT_SUCCESS);
    }

    // Получение пути
    char progpath[2048];
    ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
    if (len == -1) {
        const char msg[] = "Failed to read /proc/self/exe\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    while (progpath[len] != '/')
        --len;
    progpath[len] = '\0';

    // Создание pipe
    int parent_to_child[2];
    int child_to_parent[2];
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        const char msg[] = "Failed to create pipes\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        const char msg[] = "Failed to fork\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочерний процесс
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        dup2(parent_to_child[0], STDIN_FILENO);
        dup2(child_to_parent[1], STDOUT_FILENO);

        close(parent_to_child[0]);
        close(child_to_parent[1]);

        char path[4096];
        snprintf(path, sizeof(path) - 1, "%s/%s", progpath, CHILD_PROGRAM_NAME);

        char *const args[] = {CHILD_PROGRAM_NAME, argv[1], NULL};
        execv(path, args);

        const char msg[] = "Failed to exec child\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    close(parent_to_child[0]);
    close(child_to_parent[1]);

    char input_buf[1024];
    ssize_t bytes = read(STDIN_FILENO, input_buf, sizeof(input_buf));
    if (bytes < 0) {
        const char msg[] = "Failed to read from stdin\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    write(parent_to_child[1], input_buf, bytes);
    close(parent_to_child[1]); // EOF для ребёнка

    char result_buf[256];
    ssize_t result_bytes =
        read(child_to_parent[0], result_buf, sizeof(result_buf));
    if (result_bytes > 0) {
        write(STDOUT_FILENO, result_buf, result_bytes);
    } else {
        const char msg[] = "No result from child\n";
        write(STDERR_FILENO, msg, sizeof(msg));
    }

    close(child_to_parent[0]);

    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}
