#if !defined(UTILS_H)
#define UTILS_H
#include <stdbool.h>

void assert(const bool assertion, const char *s, ...);
void panic(const char* s, ...);
#endif
