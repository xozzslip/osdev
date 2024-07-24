#include <stdbool.h>

bool streq(const char *a, const char *b) {
    int i = 0;
    while (true) {
        if (a[i] != b[i]) {
            return false;
        }
        if (a[i] == '\0') {
            return true;
        }
        i++;
    }
}
