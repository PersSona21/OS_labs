#include "library.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// указатели на функции
static gcd_func_t *gcd_ptr = NULL;
static convert_func_t *convert_ptr = NULL;

// дискрипторы библиотек
static void *handle_gcd = NULL;
static void *handle_conv = NULL;

static int current = 1; // 1 или 2

// заглушки
static int gcd_stub(int a, int b) {
    (void)a;
    (void)b;
    return 0;
}

static char *convert_stub(int x) {
    (void)x;
    return strdup("error: library not loaded");
}

void load_libraries(void) {
    char path_g[64];
    char path_c[64];

    // формируем имена библиотек
    snprintf(path_g, sizeof(path_g), "./libgcd%d.so", current);
    snprintf(path_c, sizeof(path_c), "./libconvert%d.so", current);

    // закрываем библиотеки если они были открыты ранее
    if (handle_gcd)
        dlclose(handle_gcd);
    if (handle_conv)
        dlclose(handle_conv);

    // загружаем библиотеки
    handle_gcd = dlopen(path_g, RTLD_NOW | RTLD_LOCAL);
    handle_conv = dlopen(path_c, RTLD_NOW | RTLD_LOCAL);

    // пытаемся найти функции gcd/convert
    gcd_ptr = handle_gcd ? dlsym(handle_gcd, "gcd") : NULL;
    convert_ptr = handle_conv ? dlsym(handle_conv, "convert") : NULL;

    // если не нашли используем заглушки
    if (!gcd_ptr)
        gcd_ptr = gcd_stub;
    if (!convert_ptr)
        convert_ptr = convert_stub;
}

int main(void) {
    load_libraries();

    char line[256];

    while (fgets(line, sizeof(line), stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1)
            continue;

        if (cmd == 0) {
            current = 3 - current;
            load_libraries();
            printf("Switched to implementation %d\n", current);
            continue;
        }

        if (cmd == 1) {
            int a, b;
            if (sscanf(line, "%*d %d %d", &a, &b) == 2) {
                printf("%d\n", gcd_ptr(a, b));
            }
        } else if (cmd == 2) {
            int x;
            if (sscanf(line, "%*d %d", &x) == 1) {
                char *res = convert_ptr(x);
                if (res) {
                    printf("%s\n", res);
                    free(res);
                } else {
                    printf("error\n");
                }
            }
        }
    }

    if (handle_gcd)
        dlclose(handle_gcd);
    if (handle_conv)
        dlclose(handle_conv);

    return 0;
}