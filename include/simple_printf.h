
#ifndef SIMPLE_PRINTF_H_
#define SIMPLE_PRINTF_H_

#if defined(USE_SIMPLE_PRINTF)
int simple_sprintf(char *out, const char *format, ...);
int printf(const char *format, ...);
#define printf(format,...) printf(format,## __VA_ARGS__ )
#else
#define printf(format,...)
#endif

#endif /* SIMPLE_PRINTF_H_ */
