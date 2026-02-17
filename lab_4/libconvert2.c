#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *convert(int x) {
    if (x == 0) {
        char *s = malloc(2);
        if (!s) {
            fprintf(stderr, "ERROR: unable to allocate memory \n");
            exit(EXIT_FAILURE);
        }
        strcpy(s, "0");
        return s;
    }

    char negative = (x < 0);
    int num = negative ? (-x) : x;

    char buf[65];
    int len = 0;

    while (num > 0) {
        buf[len++] = '0' + (num % 3);
        num /= 3;
    }

    char *result = malloc(len + negative + 1);
    if (!result)
        return NULL;

    int pos = 0;

    if (negative)
        result[pos++] = '-';

    for (int i = len - 1; i >= 0; i--)
        result[pos++] = buf[i];

    result[pos] = '\0';

    return result;
}