#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        const char msg[] = "ERROR: filename argument missing\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        const char msg[] = "ERROR: cannot open file\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char buf[1024];
    ssize_t bytes = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (bytes <= 0) {
        const char msg[] = "ERROR: no input\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        close(fd);
        exit(EXIT_FAILURE);
    }
    buf[bytes] = '\0';

    // парсим строку
    float sum = 0.0;
    char *p = buf;
    while (*p) {
        while (*p && (*p == ' ' || *p == '\n' || *p == '\t'))
            p++;
        if (!*p)
            break;

        char *end;
        float val = strtof(p, &end);
        if (p == end)
            break;
        sum += val;
        p = end;
    }

    char result[128];
    int len = snprintf(result, sizeof(result), "%.3f\n", sum);

    write(fd, result, len);            // в файл
    write(STDOUT_FILENO, result, len); // родителю
    close(fd);

    exit(EXIT_SUCCESS);
}
