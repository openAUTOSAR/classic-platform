
#ifndef SIMPLE_PRINTF_H_
#define SIMPLE_PRINTF_H_

#if defined(USE_SIMPLE_PRINTF)
int simple_sprintf(char *out, const char *format, ...);
int simple_printf(const char *format, ...);
#define simple_printf(format,...) simple_printf(format,## __VA_ARGS__ )
#else
#define simple_printf(format,...)
#endif

#endif /* SIMPLE_PRINTF_H_ */
