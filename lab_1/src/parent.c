#include <mach-o/dyld.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char *argv[]) {

    char filename[512]; // читаем название текстового файла
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        const char msg[] = "Error: failed to read filename\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        return 1;
    }

    filename[strcspn(filename, "\n")] = '\0';

    if (filename[0] == '\0') {
        const char msg[] = "Error: filename cannot be empty\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        return 1;
    }

    // Получение пути директории
    char progpath[2048];
    uint32_t size = sizeof(progpath);
    if (_NSGetExecutablePath(progpath, &size) != 0) {
        const char msg[] = "Failed to get executable path\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    ssize_t len = strlen(progpath);
    while (progpath[len] != '/') {
        --len;
    }
    progpath[len] = '\0';

    // Создание pipe
    int parent_to_child[2]; // 0 - чтение, 1 - запись
    int child_to_parent[2];
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        const char msg[] = "Failed to create pipes\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork(); // > 0 значит родитель
    if (pid == -1) {
        const char msg[] = "Failed to fork\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочерний процесс
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        dup2(parent_to_child[0],
             STDIN_FILENO); // настраиваем файловые дискрипторы
        dup2(child_to_parent[1], STDOUT_FILENO);

        close(parent_to_child[0]);
        close(child_to_parent[1]);

        char path[4096];
        snprintf(path, sizeof(path) - 1, "%s/%s", progpath, CHILD_PROGRAM_NAME);

        char *const args[] = {CHILD_PROGRAM_NAME, filename, NULL};
        execv(path, args); // заменяем код на код из child

        const char msg[] = "Failed to exec child\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    // pid > 0 (в родителе)
    close(parent_to_child[0]);
    close(child_to_parent[1]);

    char input_buf[1024];
    ssize_t bytes = read(STDIN_FILENO, input_buf,
                         sizeof(input_buf)); // читаем данные с консоли
    if (bytes < 0) {
        const char msg[] = "Failed to read from stdin\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    write(parent_to_child[1], input_buf,
          bytes);              // передаём данные дочернему процессу
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
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { // успешное окончание
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}
