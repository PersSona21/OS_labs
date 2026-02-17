#include "library.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char line[256];

    while (fgets(line, sizeof(line), stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1)
            continue;

        if (cmd == 1) {
            int a, b;
            if (sscanf(line, "%*d %d %d", &a, &b) == 2) {
                printf("%d\n", gcd(a, b));
            }
        } else if (cmd == 2) {
            int x;
            if (sscanf(line, "%*d %d", &x) == 1) {
                char *res = convert(x);
                if (res) {
                    printf("%s\n", res);
                    free(res);
                } else {
                    printf("error\n");
                }
            }
        }
    }

    return 0;
}