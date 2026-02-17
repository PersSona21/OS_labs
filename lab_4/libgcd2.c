#include <stdlib.h>

int gcd(int a, int b) {
    a = abs(a);
    b = abs(b);
    if (a == 0)
        return b ? b : 1;
    if (b == 0)
        return a;

    int min = a < b ? a : b;
    for (int d = min; d >= 1; d--) {
        if (a % d == 0 && b % d == 0) {
            return d;
        }
    }
    return 1;
}