#ifndef LIBRARY_H
#define LIBRARY_H

/* Объявления функций (для статической линковки и компиляции) */
int gcd(int a, int b);
char *convert(int x);

/* Типы указателей на функции (для динамической загрузки) */
typedef int gcd_func_t(int a, int b);
typedef char *convert_func_t(int x);

#endif